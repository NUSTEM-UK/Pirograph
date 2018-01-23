import paho.mqtt.client as mqtt
from time import sleep

mqttc = mqtt.Client()
mqtt_server = "10.0.1.5"

def message(topic, payload):
    mqttc.connect(mqtt_server, 1883)
    mqttc.publish("pirograph/"+topic, payload)

message("test", "testing")
