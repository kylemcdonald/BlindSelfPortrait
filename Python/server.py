import bottle
bottle.BaseRequest.MEMFILE_MAX = 5_000_000 # bytes
from bottle import route, run, request, response
from PIL import Image
import notebooks.blind_contour.cld_mst as process_cld

import io 
import re
import base64
import ujson

@route('/')
def index():
    return """
<!DOCTYPE html5>
<head>
<title>Hello</title>
</head>
<body>
<script src="https://cdnjs.cloudflare.com/ajax/libs/geojson2svg/1.2.3/geojson2svg.min.js"></script>
<video autoplay="true" id="webcam"></video>
<div id="status">press spacebar to process</div>

<select id="mode">
  <option value="steiner" selected>steiner</option>
  <option value="astar">astar</option>
</select>

<svg id="result" width="256" height="256" viewbox="0 -256 256 256" xmlns="http://www.w3.org/2000/svg"></svg>
<pre>
APIs:
http://this ip:8989/canny_cld_steiner
http://this ip:8989/canny_cld_astar

POST a JSON of form
{
    imgdata: base64 dataurl
}
RETURNS:
a GeoJSON LineString geometry


POST a JSON of form
{
    imgdata: base64 dataurl
}
RETURNS:
a GeoJSON LineString geometry

</pre>
<script  type="text/javascript">

function downloadSVG(svgData, method) {
    const svgBlob = new Blob([svgData], {type:"image/svg+xml;charset=utf-8"});
    const svgUrl = URL.createObjectURL(svgBlob);
    const downloadLink = document.createElement("a");
    downloadLink.href = svgUrl;
    downloadLink.download = `contour_${method}_${Date.now()}.svg`;
    document.body.appendChild(downloadLink);
    downloadLink.click();
    document.body.removeChild(downloadLink);

}

async function main() {
    const video = document.querySelector('#webcam')
    const status = document.querySelector('#status')
    const output = document.querySelector('#result')
    const mode = document.querySelector('#mode')
    const converter = geojson2svg({ 
        mapExtent: { left:0, right:256, top: 0, bottom: 256 },
        attributes: { "stroke": "black", "fill": "none", "stroke-width":"1.5px" }
    })
    try {
        const stream = await navigator.mediaDevices.getUserMedia({ video: true })
        video.srcObject = stream
    } catch (ex) {
        console.log(ex)
    }

    const canvas = document.createElement('canvas')
    document.body.appendChild(canvas)
    await new Promise((res, rej) => {
        const interval = setInterval(() => {
            if(video.videoWidth != 0) {
                clearInterval(interval)
                res()
            }
        }, 10)
    })

    canvas.width  = 256
    canvas.height = 256

    const ctx = canvas.getContext('2d')
    let lastOutput = undefined
    const process = async () => {
        console.log('processing...')
        const vw = video.videoWidth
        const vh = video.videoHeight
        const aspect = vw/vh
        const offset = (aspect - 1.0) * vw
        ctx.drawImage(video, -offset/4, 0, 256 + offset/2, 256)
        
        const imgdata = canvas.toDataURL()
        status.innerText = 'uploading...'
        const config = {
            method: 'POST',
            headers: {
                'Accept': 'application/json',
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ imgdata })
        }
        const response = await fetch(`http://${window.location.host}/canny_cld_${mode.value}`, config)
        status.innerText = 'got response'
        try {
            const json = await response.json()
            const coords = json['coordinates']
            
            const svgBits = converter.convert(json)
            output.innerHTML = svgBits[0]
            downloadSVG(output.outerHTML, mode.value)
            status.innerText = 'done. see console for return value'
        } catch (err) {
            const txt = await response.text()
            console.error(err, txt)
            status.innerText = `error: ${txt}, ${err}, see consode for more details`
        }
    }



    document.addEventListener("keydown", e => {
        if(e.code == 'Space') process()
    })
}

main()

</script>
</body>
"""

@route('/canny_cld_astar', method='POST')
def do_upload():
    j = request.json
    try:
        imgdata = j['imgdata']
    except KeyError:
        print("no img data found")
        return ""

    try:
        encoded = re.sub('^data:image/.+;base64,', '', imgdata)
        data = base64.b64decode(encoded, validate=True)
        b = io.BytesIO(data)
        img = Image.open(b)
    except Exception as e:
        print(e)
        return ""
    ret = process_cld.rgb2line(img)
    response.content_type = 'application/json'
    return ujson.dumps(ret)

@route('/canny_cld_steiner', method='POST')
def do_upload():
    j = request.json
    try:
        imgdata = j['imgdata']
    except KeyError:
        print("no img data found")
        return ""

    try:
        encoded = re.sub('^data:image/.+;base64,', '', imgdata)
        data = base64.b64decode(encoded, validate=True)
        b = io.BytesIO(data)
        img = Image.open(b)
    except Exception as e:
        print(e)
        return ""
    ret = process_cld.rgb2line_steiner(img)
    response.content_type = 'application/json'
    return ujson.dumps(ret)


run(host='0.0.0.0', port=8989)
