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
from colorsys import rgb_to_hsv # Needed for Hex to HSV conversions

mqttc = mqtt.Client()

# TODO: expose interface for configuring the MQTT broker address & port
mqtt_server = "10.0.1.3"
# mqtt_server = "localhost"
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

def hex_to_rgb(value):
    """Helper function to handle hex colour codes, and return rgb.
       
       Used with colorsys routines to extract hue angle from web hex data.
    """
    value = value.lstrip('#')
    lv = len(value)
    return tuple(int(value[i:i + lv // 3], 16) for i in range(0, lv, lv // 3))

class Skutter:
    """Initial implementation of Skutter type.

    """

    skutterDict = {}

    def __init__(self, skutterNumber="00", requestedLEDcount="1"):
        """Initialise the skutter, with vaguely-sane defaults."""
        self._mac = MACS[skutterNumber] # MAC address of specific Wemos module.
        self._permittedTransitionTypes = ("ONCE", "LOOP", "RETURN")
        self._cameraRotationTime = 10.0 # Time in secs (float).
        # Make sure all the variables exist
        self._LEDcount = requestedLEDcount # How many LEDs in the strip?
        self._servo1positionA = 90.0
        self._servo1positionB = 90.0
        self._servo2positionA = 90.0
        self._servo2positionB = 90.0
        self._servo1speedA = 90.0
        self._servo1speedB = 90.0
        self._servo2speedA = 90.0
        self._servo2speedB = 90.0
        self._LEDstartHueA = 0
        self._LEDstartHueB = 0
        self._LEDendHueA = 0
        self._LEDendHueB = 0
        self._transitionTime = 5.0
        self._stepper1speedA = 0
        self._stepper1speedB = 0
        self._stepper2speedA = 0
        self._stepper2speedB = 0
        self._stepper1angleA = 0
        self._stepper1angleB = 0
        self._stepper2angleA = 0
        self._stepper2angleB = 0


    def _message(self, payload, topic=""):
        """Fire the MQTT message (internal class method)
        
        Receives payload as dictionary, topic optional
        """
        # Package up the data
        mqttc.connect(mqtt_server, mqtt_port)
        mqttc.publish(mqtt_channel+topic, json.dumps(payload))
        # mqttc.publish(mqtt_channel+topic, self.jsonTestData)


    @property
    def mac(self):
        """Output object MAC address, as string. For testing purposes."""
        return self._mac

    def TestSend(self):
        testDict = {"name": "Bob", "age": 42}
        # self._message(testDict, "test")
        self._message(testDict, self._mac)
    
    def pitch(self, movement):
        messageDict = {"command": "setPitch", "value": movement}
        self._message(messageDict, self._mac)

    def roll(self, movement):
        messageDict = {"command": "setRoll", "value": movement}
        self._message(messageDict, self._mac)

    def setLEDhue(self, position, state, value):
        """Command LED colour change."""
        # TODO: sanity check on inputs
        if value.startswith("#"):
            # We have a hex code - convert to HSV (via RGB) and send Hue
            r, g, b = hex_to_rgb(value)
            h, s, v = rgb_to_hsv(r, g, b)
            # Now send the hue value, scaled to integer 0-255 (which the Skutter expects)
            messageDict = {"command": "setLEDhue", "position": position, "state": state, "H": int(h*255), "S": int(s*255), "V": int(v*255)}
        else:
            # Assume it's hue value and cast to int
            messageDict = {"command": "setLEDhue", "position": position, "state": state, "value": int(value)}
        self._message(messageDict, self._mac)

    def LEDstartHue(self, targetHue):
        """set LED hue for first pixel, for states A and B."""
        # TODO: sanity check on inputs
        self.setLEDhue("start", "A", targetHue)
        self.setLEDhue("start", "B", targetHue)

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
    
    @property
    def LEDstartHueA(self):
        return self._LEDstartHueA

    @LEDstartHueA.setter
    def LEDstartHueA(self, targetHue):
        """set LED hue for first pixel, for state A."""
        # TODO: sanity check on inputs
        self._LEDstartHueA = targetHue
        # Cast targetHue to string so setLEDhue handler can process, in case it isn't a string.
        self.setLEDhue("start", "A", str(targetHue))

    @property
    def LEDstartHueB(self):
        return self._LEDstartHueB

    @LEDstartHueB.setter
    def LEDstartHueB(self, targetHue):
        """set LED hue for first pixel, for state B."""
        # TODO: sanity check on inputs
        self._LEDstartHueB = targetHue
        self.setLEDhue("start", "B", str(targetHue))
    
    @property
    def LEDendHueA(self):
        return self._LEDendHueA

    @LEDendHueA.setter
    def LEDendHueA(self, targetHue):
        """set LED hue for last pixel, for state A"""
        # TODO: sanity check on inputs
        self._LEDendHueA = targetHue
        self.setLEDhue("end", "A", str(targetHue))
    
    @property
    def LEDendHueB(self):
        return self._LEDendHueB

    @LEDendHueB.setter
    def LEDendHueB(self, targetHue):
        """set LED hue for last pixel, for state B."""
        # TODO: sanity check on inputs
        self._LEDendHueB = targetHue
        self.setLEDhue("end", "B", str(targetHue))
    
    def setServoPosition(self, servoNum, state, angle):
        """base message sender for servo interactions."""
        # TODO: sanity check on inputs
        messageDict = {"command": "setServoPosition", "servoNum": servoNum, "state": state, "angle": int(angle)}
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
        self.servo1position(targetSpeed - 90)

    def servo2speed(self, targetSpeed):
        """Convenience alias for setServo2position."""
        self.servo2position(targetSpeed - 90)

    @property
    def servo1positionA(self):
        return self._servo1positionA

    @servo1positionA.setter
    def servo1positionA(self, targetSpeed):
        self._servo1positionA = targetSpeed
        # setattr(self, 'setServoPosition', "1", "A", targetSpeed)
        self.setServoPosition("1", "A", targetSpeed)
    
    @property
    def servo1positionB(self):
        return self._servo1positionB

    @servo1positionB.setter
    def servo1positionB(self, targetSpeed):
        self._servo1positionB = targetSpeed
        self.setServoPosition("1", "B", targetSpeed)

    @property
    def servo2positionA(self):
        return self._servo2positionA

    @servo2positionA.setter
    def servo2positionA(self, targetSpeed):
        self._servo2positionA = targetSpeed
        self.setServoPosition("2", "A", targetSpeed)

    @property
    def servo2positionB(self):
        return self._servo2positionB
    
    @servo2positionB.setter
    def servo2positionB(self, targetSpeed):
        self._servo2positionB = targetSpeed
        self.setServoPosition("2", "B", targetSpeed)

    @property
    def servo1speedA(self):
        return str(int(self._servo1speedA) - 90)

    @servo1speedA.setter
    def servo1speedA(self, targetSpeed):
        self._servo1speedA = str(int(targetSpeed) + 90)
        setattr(self, 'servo1positionA', str(int(targetSpeed) + 90))
        # self.servo1positionA(targetSpeed)

    @property
    def servo1speedB(self):
        return str(int(self._servo1speedB) - 90)

    @servo1speedB.setter    
    def servo1speedB(self, targetSpeed):
        self._servo1speedB = str(int(targetSpeed) + 90)
        setattr(self,'servo1positionB', str(int(targetSpeed) + 90))
        # self.servo1positionB(targetSpeed)

    @property
    def servo2speedA(self):
        return str(int(self._servo2speedA) - 90)

    @servo2speedA.setter
    def servo2speedA(self, targetSpeed):
        self._servo2speedA = targetSpeed + 90
        self.servo2positionA(targetSpeed + 90)

    @property
    def servo2speedB(self):
        return str(int(self._servo2speedB) - 90)

    @servo2speedB.setter
    def servo2speedB(self, targetSpeed):
        self._servo2speedB = targetSpeed + 90
        self.servo2positionB(targetSpeed + 90)
    
    def setBrightness(self, targetBrightness):
        """Set LED pixel brightness."""
        # TODO: Sanity check on input value
        messageDict = {"command": "setBrightness", "value": targetBrightness}
        self._message(messageDict, self._mac)

    def LEDbrightness(self, targetBrightness):
        """Convenience method alias for setBrightness."""
        self.setBrightness(int(targetBrightness))

    @property
    def transitionTime(self):
        return self._transitionTime

    @transitionTime.setter
    def transitionTime(self, requested_time):
        # TODO: add sanity check
        if int(requested_time) < 0: # TODO: check if transitionTime is integer
            # TODO: raise error
            pass
        else:
            self._transitionTime = requested_time
            messageDict = {"command": "setTransitionTime", "value": int(requested_time)*1000}
            # my_dict = {'id':self._mac, 'transitionTime':requested_angle}
            self._message(messageDict, self._mac)

    @property
    def transitionType(self):
        return self._transitionType

    @transitionType.setter
    def transitionType(self, requested_type):
        """Commands colour animation change."""
        if requested_type in self.permittedTransitionTypes:
            self._transitionType = requested_type
            # sanity check - is command in the permittedTransitionTypes list?
            messageDict = {"command": "setTransitionType", "value": requested_type}
            self._message(messageDict, self._mac)

    # >>> STEPPER FUNCTIONS
    # Joe's root messaging additions first

    def setStepperSpeed(self, speed, stepperNo, state):
        """Command Stepper1 speed change"""
        messageDict = {"command": "setStepperSpeed", "stepperNum": stepperNo, "speed": speed, "state": state}
        self._message(messageDict, self._mac)

    def setStepperAngle(self, angle, stepperNo, state):
        """Command Stepper1 speed change"""
        messageDict = {"command": "setStepperAngle", "stepperNum": stepperNo, "angle": angle, "state": state}
        self._message(messageDict, self._mac)

    # ...and now some getter/setter methods

    @property
    def stepper1speedA(self):
        return self._stepper1speedA

    @stepper1speedA.setter
    def stepper1speedA(self, targetSpeed):
        self._stepper1speedA = targetSpeed
        self.setStepperSpeed(targetSpeed, "1", "A")
    
    @property
    def stepper1speedB(self):
        return self._stepper1speedB

    @stepper1speedB.setter
    def stepper1speedB(self, targetSpeed):
        self._stepper1speedB = targetSpeed
        self.setStepperSpeed(targetSpeed, "1", "B")

    @property
    def stepper2speedA(self):
        return self._stepper2speedA
    
    @stepper2speedA.setter
    def stepper2speedA(self, targetSpeed):
        self._stepper2speedA = targetSpeed
        self.setStepperSpeed(targetSpeed, "2", "A")
    
    @property
    def stepper2speedB(self):
        return self._stepper2speedB

    @stepper2speedB.setter
    def stepper2speedB(self, targetSpeed):
        self._stepper2speedB = targetSpeed
        self.setStepperSpeed(targetSpeed, "2", "B")

    @property
    def stepper1angleA(self):
        return self._stepper1angleA
    
    @stepper1angleA.setter
    def stepper1angleA(self, targetAngle):
        self._stepper1angleA = targetAngle
        self.setStepperAngle(targetAngle, "1", "A")
    
    @property
    def stepper1angleB(self):
        return self._stepper1angleB
    
    @stepper1angleB.setter
    def stepper1angleB(self, targetAngle):
        self._stepper1angleB = targetAngle
        self.setStepperAngle(targetAngle, "1", "B")

    @property
    def stepper2angleA(self):
        return self._stepper2angleA
    
    @stepper2angleA.setter
    def stepper2angleA(self, targetAngle):
        self._stepper2angleA = targetAngle
        self.setStepperAngle(targetAngle, "2", "A")
    
    @property
    def stepper2angleB(self):
        return self._stepper2angleB
    
    @stepper2angleB.setter
    def stepper2angleB(self, targetAngle):
        self._stepper2angleB = targetAngle
        self.setStepperAngle(targetAngle, "2", "B")


if __name__ == '__main__':
    my_skutter = Skutter("D10")
    my_MAC = my_skutter.getMac()
    print(my_MAC)