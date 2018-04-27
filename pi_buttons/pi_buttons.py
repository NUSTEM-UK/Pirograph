"""Handle buttons presses to send reset and save commands to Processing sketch.

Run alongside the Processing viewer sketch for the control panel Pi installations.

Usage: python3 pi_buttons.py 0
       (for channel 0 / A)
       
Don't forget to `pip3 install paho-mqtt`.
"""

import sys
import paho.mqtt.client as mqtt
from gpiozero import Button
from signal import pause

def message(payload, topic=""):
    """Send the MQTT message.
    
    Not doing JSON for this little script.
    """
    # Package up the data
    mqttc.connect(mqtt_server, mqtt_port)
    mqttc.publish(mqtt_channel+topic, payload)
    print("Sending: " + topic + " : " + payload)
    
def messageReset():
    message(THISPORT, "reset")
    
def messageSave():
    message(THISPORT, "save")
    
buttonReset = Button(17)
buttonSave = Button(27)

mqttc = mqtt.Client()

mqtt_server = "10.0.1.3"
# mqtt_server = "127.0.0.1"

mqtt_port = 1883
mqtt_channel = "pirograph/"


THISPORT = sys.argv[1]
print("Handling button presses for ", THISPORT)

# Attach callback functions
buttonReset.when_released = messageReset
buttonSave.when_released = messageSave

# Now wait for things to happen
pause()