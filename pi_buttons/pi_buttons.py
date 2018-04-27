"""Handle buttons presses to send reset and save commands to Processing sketch.

Run alongside the Processing viewer sketch for the control panel Pi installations.

Usage: python3 pi_buttons.py 0
       (for channel 0 / A)
"""

import sys
import paho.mqtt.client as mqtt
from gpiozero import Button
from signal import pause

buttonReset = Button(2)
buttonSave = Button(3)

mqttc = mqtt.Client()

# mqtt_server = "10.0.1.3"
mqtt_server = "127.0.0.1"
mqtt_port = 1883
mqtt_channel = "pirograph"

def message(payload, topic=""):
    """Send the MQTT message.
    
    Not doing JSON for this little script.
    """
    # Package up the data
    mqttc.connect(mqtt_server, mqtt_port)
    mqttc.publish(mqtt_channel+topic, payload)

THISPORT = sys.argv[1]
print("Handling button presses for ", THISPORT)

# Attach callback functions
buttonReset.whenReleased = message(THISPORT, "reset")
buttonSave.whenReleased = message(THISPORT, "save")

# Now wait for things to happen
pause()