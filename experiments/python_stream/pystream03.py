"""Load images from MotionEyeOS MJPEG stream into a Pillow Image type.

This test code derived from: https://stackoverflow.com/questions/21702477/how-to-parse-mjpeg-http-stream-from-ip-camera
"""

import requests
from PIL import Image
from io import BytesIO

r = requests.get('http://10.0.1.10:8081', stream=True)

if (r.status_code == 200):
    bytes = bytes()
    for chunk in r.iter_content(chunk_size=1024):
        # TODO: Explore performance with different chunk sizes in above line
        bytes += chunk
        a = bytes.find(b'\xff\xd8')
        b = bytes.find(b'\xff\xd9')
        if a != -1 and b != -1:
            jpg = bytes[a:b+2]
            bytes = bytes[b+2:]
            img = Image.open(BytesIO(jpg))
            print(img.format, img.size, img.mode)
            img.show()
else:
    print("Received unexpected status code {}".format(r.status_code))
