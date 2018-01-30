"""Minimal-boilerplate interface to remove robots, which for historical
reasons we refer to as 'Skutters'.

Skutters are implemented on Wemos D1 mini boards, with message-passing via
JSON packaged over MQTT. See elsewhere in this repo for the Wemos/Arduino
code, which is where most of the heavy lifting is done. This Python layer
is mostly an abstraction of the message-passing and networking code.
"""

import json
import paho.mqtt.client as mqtt
from skutter_mac import MACS

mqttc = mqtt.Client()

# TODO: expose interface for configuring the MQTT broker address & port
mqtt_server = "10.0.1.3"
# mqtt_server = "localhost"
mqtt_port = 1883
# TODO: expose interface for configuring the MQTT channel root
mqtt_channel = "pirograph/"

# TODO: import list of Wemos module MAC addresses, as dictionary

""" TODO: when a skutter powers up, it should announce itself to pirograph/skutter
with its MAC address. A handler here should trigger, check that MAC against the
dictionary, add the MAC to the dict if necessary (and save the dict), then reply
to the skutter giving it a handle/serial number.

The main use of this is to allow a construction like:

    my_skutter = Skutter("D15")

...which would instantiate a new Skutter object attached to the device physically
labelled 'D15'.

The simpler way of doing this is to have a separate routine which preallocates the
labels for every device we're going to power up. Which saves a bunch of logic at
runtime, without a huge compromise (that is, the skutter list isn't dynamic during
execution, but we can probably live with that).
"""

class Skutter:
    """Initial implementation of Skutter type.

    """

    skutterDict = {}

    def __init__(self, skutterNumber="00", requestedLEDcount="1"):
        """Initialise the skutter, with vaguely-sane defaults."""
        self._mac = MACS[skutterNumber] # MAC address of specific Wemos module.
        self.transitionTime = 90 # Express in degrees? Is that the simplest way here?
        self.transitionType = "interpolate" # pick a default. There may be others.
        self.permittedTransitionTypes = ("interpolate", "sine", "square")
        self.cameraRotationTime = 10.0 # Time in secs (float).
        # Make sure all the variables exist
        self.servo1position = 90.0
        self.servo2position = 90.0
        self.LEDstartHue = 0.0
        self.LEDendHue = 0.0
        self.LEDcount = requestedLEDcount # How many LEDs in the strip?
        self.LEDbrightness = 50 # percentage
        # Package the data.
        

    def _dictionarify(self):
        """Package everything up into a neat dictionary, for JSONificaation."""
        # self.skutterDict["mac"] = self._mac
        self.skutterDict["transitionTime"] = self.transitionTime
        self.skutterDict["transitionType"] = self.transitionType
        self.skutterDict["cameraRotationTime"] = self.cameraRotationTime
        self.skutterDict["servo1position"] = self.servo1position
        self.skutterDict["servo2position"] = self.servo2position
        self.skutterDict["LEDstartHue"] = self.LEDstartHue
        self.skutterDict["LEDendHue"] = self.LEDendHue
        self.skutterDict["LEDcount"] = self.LEDcount
        self.skutterDict["LEDbrightness"] = self.LEDbrightness
        # Leave a stub here for the next thing I think to add.
        # self.skutterDict[""] = self.

    def _message(self, payload, topic=""):
        """Fire the MQTT message (internal class method)
        
        Receives payload as dictionary, topic optional
        """
        # Package up the data
        mqttc.connect(mqtt_server, mqtt_port)
        mqttc.publish(mqtt_channel+topic, json.dumps(payload))
        # mqttc.publish(mqtt_channel+topic, self.jsonTestData)

    def getMac(self):
        """Output object MAC address, as string. For testing purposes."""
        return self._mac

    def TestSend(self):
        testDict = {"name": "Bob", "age": 42}
        # self._message(testDict, "test")
        self._message(testDict, self._mac)

    def LEDstartColour(self, targetColour):
        """Set target colour for the first pixel."""
        # TODO: Sanity check on input value
        messageDict = {"command": "LEDstartHue", "value": targetColour}
        self._message(messageDict, self._mac)

    def LEDendColour(self, targetColour):
        """Set target colour for the last pixel."""
        # TODO: Sanity check on input value
        messageDict = {"command": "LEDendHue", "value": targetColour}
        self._message(messageDict, self._mac)

    def LEDcolour(self, targetColour):
        """Convenience function to set the colour of all LEDs."""
        # TODO: sanity check on inputs
        # TODO: Actually set all LEDs rather than just the first
        self.LEDstartColour(targetColour)

    def servo1position(self, targetSpeed):
        """Set target speed of servo 1."""
        # TODO: Sanity check on input value
        messageDict = {"command": "servo1position", "value": targetspeed}
        self._message(messageDict, self._mac)
    
    def setBrightness(self, targetBrightness):
        """Set LED pixel brightness."""
        # TODO: Sanity check on input value
        messageDict = {"command": "setBrightness", "value": targetBrightness}
        self._message(messageDict, self._mac)

    def SetTargetColour(self, position, targetColour, transitionTime, transitionType):
        """Set target colour for individual pixel."""
        self.SetTransitionTime(transitionTime)
        self.SetTransitionType(transitionType)

    def SetTargetColourStart(self, targetColour):
        """ TODO: implement."""
        # Plan here is to allow addressing pixels from position 0.0 (start of string)
        # to 1.0 (end of string), with the Wemos host interpreting the positions.
        # Needs a route to specifying whether colours should cut, blend, or wheelcycle
        # between specified colour points.
        pass

    def SetTransitionTime(self, requested_angle):
        # TODO: add sanity check
        if requested_angle < 0: # TODO: check if transitionTime is integer
            # TODO: raise error
            pass
        else:
            my_dict = {'id':self._mac, 'transitionTime':requested_angle}
            self._message(json.dumps(my_dict))

    def SetTransitionType(self, requested_type):
        """Commands colour animation change."""
        if requested_type in self.permittedTransitionTypes:
            # sanity check - is command in the permittedTransitionTypes list?
            my_dict = {'id':self._mac, 'transitionType':requested_type}
            self._message(json.dumps(my_dict))

    def SetAnimationRate(self, requested_animation_rate):
        """Commands self-running animation type."""
        # TODO: implement
        # Could rotate via angle, oscillate between colour points
        # ...which itself could be square, sine, wheel, etc.
        pass


if __name__ == '__main__':
    my_skutter = Skutter("D10")
    my_MAC = my_skutter.getMac()
    print(my_MAC)