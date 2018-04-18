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
#mqtt_server = "localhost"
# mqtt_server = "192.168.0.31"
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
        self.permittedTransitionTypes = ("ONCE", "LOOP", "RETURN")
        self.cameraRotationTime = 10.0 # Time in secs (float).
        # Make sure all the variables exist
        self.LEDcount = requestedLEDcount # How many LEDs in the strip?
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

    def setLEDhue(self, position, state, value):
        """Command LED colour change."""
        # TODO: sanity check on inputs
        messageDict = {"command": "setLEDhue", "position": position, "state": state, "value": value}
        self._message(messageDict, self._mac)

    def LEDstartHue(self, targetHue):
        """set LED hue for first pixel, for states A and B."""
        # TODO: sanity check on inputs
        self.setLEDhue("start", "A", targetHue)
        self.setLEDhue("start", "B", targetHue)

# Joe's Additions addition of a set both start and enf of Ab and then B for neos
    def LEDHueA(self, targetHue):
        """set LED hue for first pixel, for states A and B."""
        # TODO: sanity check on inputs
        self.setLEDhue("start", "A", targetHue)
        self.setLEDhue("end", "A", targetHue)
        self.setBrightness(200)

    def LEDHueB(self, targetHue):
        """set LED hue for first pixel, for states A and B."""
        # TODO: sanity check on inputs
        self.setLEDhue("start", "B", targetHue)
        self.setLEDhue("end", "B", targetHue)
        self.setBrightness(0)
# change servo for A and B together
    def testservo(self, servoNum, Aspeed, Bspeed):
        """set start, end and transitions for servo 1"""
        self.setServoPosition(servoNum,"A",Aspeed)
        self.setServoPosition(servoNum,"B",Bspeed)

# stop everything that moves
    def chill(self):
        self.setServoPosition("1","A",90)
        self.setServoPosition("1","B",90)
        self.setServoPosition("2","A",90)
        self.setServoPosition("2","B",90)


    def LEDendHue(self, targetHue):
        """set LED hue for last pixel, for states A and B."""
        # TODO: sanity check on inputs
        self.setLEDhue("end", "A", targetHue)
        self.setLEDhue("end", "B", targetHue)

    def LEDstartColour(self, targetColour):
        """Convenience alias of LEDstartHue."""
        # TODO: Sanity check on input value
        self.LEDstartHue(targetColour)

    def LEDendColour(self, targetColour):
        """Convenience alias of LEDendHue."""
        # TODO: Sanity check on input value
        self.LEDendHue(targetColour)

    def LEDcolour(self, targetColour):
        """Convenience function to set the colour of all LEDs."""
        # TODO: sanity check on inputs
        self.setLEDhue("start", "A", targetColour)
        self.setLEDhue("start", "B", targetColour)
        self.setLEDhue("end", "A", targetColour)
        self.setLEDhue("end", "B", targetColour)
    
    def LEDstartHueA(self, targetHue):
        """set LED hue for first pixel, for state A."""
        # TODO: sanity check on inputs
        self.setLEDhue("start", "A", targetHue)

    def LEDstartHueB(self, targetHue):
        """set LED hue for first pixel, for state B."""
        # TODO: sanity check on inputs
        self.setLEDhue("start", "B", targetHue)
    
    def LEDendHueA(self, targetHue):
        """set LED hue for last pixel, for state A"""
        # TODO: sanity check on inputs
        self.setLEDhue("end", "A", targetHue)
    
    def LEDendHueB(self, targetHue):
        """set LED hue for last pixel, for state B."""
        # TODO: sanity check on inputs
        self.setLEDhue("end", "B", targetHue)
    
    def setServoPosition(self, servoNum, state, angle):
        """base message sender for servo interactions."""
        # TODO: sanity check on inputs
        messageDict = {"command": "setServoPosition", "servoNum": servoNum, "state": state, "angle": angle}
        self._message(messageDict, self._mac)

    def servo1position(self, targetSpeed):
        """Set target speed of servo 1."""
        # TODO: Sanity check on input value
        self.setServoPosition("1", "A", targetSpeed)
        self.setServoPosition("1", "B", targetSpeed)
    
    def servo2position(self, targetSpeed):
        """Set target speed of servo 2."""
        # TODO: Sanity check on input value
        self.setServoPosition("2", "A", targetSpeed)
        self.setServoPosition("2", "B", targetSpeed)
    
    def servo1speed(self, targetSpeed):
        """Convenience alias for setServo1position."""
        self.servo1position(targetSpeed)

    def servo2speed(self, targetSpeed):
        """Convenience alias for setServo2position."""
        self.servo2position(targetSpeed)

    def servo1positionA(self, targetSpeed):
        self.setServoPosition("1", "A", targetSpeed)
    
    def servo1positionB(self, targetSpeed):
        self.setServoPosition("1", "B", targetSpeed)

    def servo2positionA(self, targetSpeed):
        self.setServoPosition("2", "A", targetSpeed)

    def servo2positionB(self, targetSpeed):
        self.setServoPosition("2", "B", targetSpeed)

    def servo1speedA(self, targetSpeed):
        self.servo1positionA(targetSpeed)
    
    def servo1speedB(self, targetSpeed):
        self.servo1positionB(targetSpeed)

    def servo2speedA(self, targetSpeed):
        self.servo2positionA(targetSpeed)

    def servo2speedB(self, targetSpeed):
        self.servo2positionB(targetSpeed)
    
    def setBrightness(self, targetBrightness):
        """Set LED pixel brightness."""
        # TODO: Sanity check on input value
        messageDict = {"command": "setBrightness", "value": targetBrightness}
        self._message(messageDict, self._mac)

    def LEDbrightness(self, targetBrightness):
        """Convenience method alias for setBrightness."""
        self.setBrightness(targetBrightness)

    def transitionTime(self, requested_angle):
        # TODO: add sanity check
        if requested_angle < 0: # TODO: check if transitionTime is integer
            # TODO: raise error
            pass
        else:
            messageDict = {"command": "setTransitionTime", "value": requested_angle*1000}
            # my_dict = {'id':self._mac, 'transitionTime':requested_angle}
            self._message(messageDict, self._mac)

    def transitionType(self, requested_type):
        """Commands colour animation change."""
        if requested_type in self.permittedTransitionTypes:
            # sanity check - is command in the permittedTransitionTypes list?
            messageDict = {"command": "setTransitionType", "value": requested_type}
            self._message(messageDict, self._mac)


if __name__ == '__main__':
    my_skutter = Skutter("D10")
    my_MAC = my_skutter.getMac()
    print(my_MAC)