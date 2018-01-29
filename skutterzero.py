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
# mqtt_server = "10.0.1.5"
mqtt_server = "localhost"
mqtt_port = 1883
# TODO: expose interface for configuring the MQTT channel root
mqtt_channel = "pirograph"

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

class Skutter(object):
    """Initial implementation of Skutter type.

    """

    def __init__(self, skutterNumber="00"):
        """Initialise the skutter, with vaguely-sane defaults."""
        self._mac = MACS[skutterNumber] # MAC address of specific Wemos module.
        self.transitionTime = 90 # Express in degrees? Is that the simplest way here?
        self.transitionType = "interpolate" # pick a default. There may be others.
        self.permittedTransitionTypes = ("interpolate", "sine", "square")
        self.cameraRotationTime = 10.0 # Time in secs (float).

    def _message(self, payload, topic=""):
        """Fire the MQTT message (internal class method)"""
        # TODO: check the default value of topic actually works. Python docs here end
        # up outputting the cheese shop sketch, and are more convoluted than helpful.
        mqttc.connect(mqtt_server, mqtt_port)
        mqttc.publish(mqtt_channel+"/"+topic, payload)

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
