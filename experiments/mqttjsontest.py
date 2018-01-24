import paho.mqtt.client as mqtt
import json
from time import sleep

mqttc = mqtt.Client()
mqtt_server = "10.0.1.5"

def message(topic, payload):
    mqttc.connect(mqtt_server, 1883)
    mqttc.publish("pirograph/"+topic, payload)

message("test", "testing")

# Python/JSON playing about
# From: https://code.tutsplus.com/tutorials/how-to-work-with-json-data-using-python--cms-25758

# JSON to Python dictionary
jsonData = '{"name": "Frank", "age": 39}'
jsonToPython = json.loads(jsonData)

print(jsonToPython["name"])
print(jsonToPython["age"])

message("test", jsonData)

# Python dictionary to JSON
# Note: JSON can store lists, dictionaries, booleans, numbers, character strings.
pythonDictionary = {'name':'Bob', 'age':'44', 'isEmployed':True}
print(pythonDictionary['name'], pythonDictionary['isEmployed'])
dictionaryToJson = json.dumps(pythonDictionary)

message("test", dictionaryToJson)