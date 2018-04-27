# https://stackoverflow.com/questions/32853980/temporarily-retrieve-an-image-using-the-requests-library

from PIL import Image
import urllib.request

url="http://10.0.1.10:8081"
# url = "http://10.0.1.10:8081/?_=1524583101941"

print(">>> Getting URL")
im = Image.open(urllib.request.urlopen(url))
# response = requests.get(url)
# response.raw.decode_content = True
print("<<< GOT URL")
print(im.format, im.mode, im.size)

# img = Image.open(response.content)
# incoming = BytesIO(response.content)
# data = incoming.read(100)
# print(BytesIO(data))

