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
<video autoplay="true" id="webcam"></video>
<div id="status">press spacebar to process</div>
<canvas id="result" width="256" height="256"></canvas>
<pre>
API:

http://<this ip>:8989/canny_cld_astar
POST a JSON of form
{
    imgdata: base64 dataurl
}
RETURNS:
a GeoJSON LineString geometry

</pre>
<script  type="text/javascript">

async function main() {
    const video = document.querySelector("#webcam")
    const status = document.querySelector('#status')
    const output = document.querySelector('#result')
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
    const outputCtx = output.getContext('2d')
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

        const response = await fetch(`http://${window.location.host}/canny_cld_astar`, config)
        status.innerText = 'got response'
        const json = await response.json()
        const coords = json['coordinates']
        
        outputCtx.clearRect(0, 0, 256, 256)
        outputCtx.beginPath()
        outputCtx.moveTo(Math.floor(coords[0][0]), Math.floor(255 - coords[0][1]))
        for(const coord of coords) {
            outputCtx.lineTo(Math.floor(coord[0]), Math.floor(255 - coord[1]))
        }
        outputCtx.stroke()

        status.innerText = 'done. see console for return value'
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

run(host='0.0.0.0', port=8989)
