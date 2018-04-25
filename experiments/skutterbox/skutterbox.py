"""This code enables younger users and users with diasbliaties to access the Pirograph Skutters 
without the use of our web app. It uses an MCP3008 details of wiring and setup can be found here: 
http://raspi.tv/2016/using-mcp3008-to-measure-temperature-with-gpio-zero-and-raspio-pro-hat
REMEMBER to enable SPI
The Neoxpixel setup can be found here https://learn.adafruit.com/neopixels-on-raspberry-pi/software"""

#import the MCP3008 from gpiozero
from gpiozero import MCP3008, Button
from skutterzero import Skutter
from neopixel import *
import colorsys

# LED strip configuration:
LED_COUNT      = 9      # Number of LED pixels.
LED_PIN        = 18      # GPIO pin connected to the pixels (18 uses PWM!).
LED_FREQ_HZ    = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA        = 10      # DMA channel to use for generating signal (try 10)
LED_BRIGHTNESS = 255     # Set to 0 for darkest and 255 for brightest
LED_INVERT     = False   # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL    = 0       # set to '1' for GPIOs 13, 19, 41, 45 or 53

# define the Analog input channels
s1speed = 0
s2angle = 1
sColour = 2
eColour = 3
tTime = 4

# neo strip array
LEDS = [7]

# define the two buttons
stateButton = Button(2)
sendButton = Button(3)

# the state holders so that we can act on button presses
sendToggle = 1
stateToggle = 1

#sendButton.is_pressed == True (State A), False (State B)
stateToggle = sendButton.is_pressed


# arrays to hold the current values for state A and B
stateAvals = [0,0,0,0]
stateBvals = [0,0,0,0]
tranTime = 5

# choose your skutter carefully
box = Skutter("??")

# setup for the Neopixels
strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)
# Intialize the library (must be called once before other functions).
strip.begin()

def allSend():
    # grab the transition time value and map
    tranTime = mapper((MCP3008(channel=4, device=0),4)
    # send the data currently in the arrays, and reset the toggle to false
    # first motor speed
    box.servo1speedA(stateAvals[0])
    box.servo1speedB(stateBvals[0])
    #second motor angles
    box.servo2positionA(stateAvals[1])
    box.servo2positionB(stateBvals[1])
    # start LED hues
    box.LEDstartHueA(stateAvals[2])
    box.LEDstartHueB(stateBvals[2])
    # end LED hues
    box.LEDendHueA(stateAvals[3])
    box.LEDendHueB(stateBvals[3])
    # transition time
    box.transitionTime(tranTime)

#function to map an analog input (0-100, to the relevant output)
def mapper(x, analogType):
    if analogType == 0:
        # speed input
        mappedVal = int((x - 0) * (1000 - 0) / (1 - 0) + 0)
    elif analogType == 1:
        # angle input
        mappedVal = int((x - 0) * (360 - 0) / (1 - 0) + 0)
    elif analogType == 2 or analogType == 3:
        # colour input
        mappedVal = int((x - 0) * (255 - 0) / (1 - 0) + 0)
    else:
        # time input
        mappedVal = int((x - 0) * (30 - 0) / (1 - 0) + 0)
    return mappedVal

while True:
    # check if the send button has been pressed, if yes send the data we have
    if sendButton.is_pressed and sendToggle == 1:
        allSend()
        # set the toggle to wait for the next press
        sendToggle = 0
    elif sendButton.is_pressed is False and sendToggle == 0:
        allSend
        # set the toggle to wait for the next press
        sendToggle = 1

    if stateButton.is_pressed and stateToggle ==1:
        stateToggle = 0
        #write the values for inputs 0-3 to the state A array
        for i in range (4):
            stateAvals[i] = mapper((MCP3008(channel=i, device=0),i)
    elif stateButton.is_pressed is False and stateToggle ==0:
        stateToggle = 0
        #write the values for inputs 0-3 to the state B array
        for i in range (4):
            stateBvals[i] = mapper((MCP3008(channel=i, device=0),i)

    # loop the colour display
    startColour = int(mapper((MCP3008(channel=sColour, device=0),sColour))
    endColour = int(mapper((MCP3008(channel=eColour, device=0),eColour))

    # map the start and end colours to create a gradient across the 8 pixel strip
    for i in range(len(LEDS)):
        gradColour = (endColour - startColour)/(7 - 0)*(i - 0) + startColour
        gradRGB = colorsys.hsv_to_rgb(gradColour,255,255)
        strip.setPixelColor(i, gradRGB)
    strip.show()




