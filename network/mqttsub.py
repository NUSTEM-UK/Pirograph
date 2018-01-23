"""Listen for (and output) commands issued via MQTT broker.

This version functions in Python3
"""
import paho.mqtt.client as mqtt


def on_connect(self, client, userdata, rc):
    """Connect to MQTT broker."""
    print("Connected with result code: " + str(rc))
    # NB. This deviates from documentation.
    # ...but it works. Unlike the docs. Raspbian Stretch issue?
    self.subscribe("#")


def on_message(client, userdata, msg):
    """Output diagnostic when message sent via broker."""
    # print("Topic:", msg.topic + '  :  Message: ' + msg.payload)
    # The above line is the documentation version... below actually works.
    print(msg.topic, msg.payload.decode('utf-8'))

client = mqtt.Client()

client.on_connect = on_connect
client.on_message = on_message

# For reasons that are completely unclear to me, connecting to 127.0.0.1
# throws a TypeError in on_connect: needs 3 arguments, four given.
# Adding `self` to the definition doesn't help. I don't understand.
client.connect('10.0.1.5', 1883)
# client.connect('127.0.0.1', 1883)
client.loop_forever()
