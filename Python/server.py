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
<pre>
API:

http://<this ip>:8989/canny_cld_astar
POST a JSON of form
{
    imgdata: <image dataURL>
}
RETURNS:
a JSON of form
{
    coords: [N x 2 array of coordinates in range 0-255]
}

</pre>
<script  type="text/javascript">

async function main() {
    const video = document.querySelector("#webcam")
    const status = document.querySelector('#status')

    try {
        const stream = await navigator.mediaDevices.getUserMedia({ video: true })
        video.srcObject = stream
    } catch (ex) {
        console.log(ex)
    }

    const canvas = document.createElement('canvas')
    
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
    
    const process = async () => {
        console.log('processing...')
        const vw = video.videoWidth
        const vh = video.videoHeight
        const f = 256 / vh
        const vwScaled = f * vw
        const offset = vwScaled - 256
        ctx.drawImage(video, -offset/2, 0, 256, 256)
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

        const response = await fetch('http://localhost:8989/canny_cld_astar', config)
        status.innerText = 'got response'
        const json = await response.json()
        console.log(json)
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

run(host='localhost', port=8989)
