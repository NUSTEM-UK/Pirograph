from skutterzero import Skutter

derek = Skutter("D07")

derek.transitionTime(5)
derek.setBrightness(255)
derek.LEDstartHueA(0)
derek.LEDstartHueB(120)
derek.LEDendHue(270)
derek.LEDstartHueA(90)

derek.servo1speed(0)
derek.servo2speed(180)
