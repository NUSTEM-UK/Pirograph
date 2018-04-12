# Python3!
# pip3 install requets

from PIL import Image
import requests

# response = requests.get("http://192.168.0.33:8081")

result = requests.get("http://quernstone.com")
print(result)

img = Image.open(requests.get("http://192.168.0.33:8081/", stream=True).raw)

img.show()