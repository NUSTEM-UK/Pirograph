from skutterzero import Skutter

derek = Skutter("D07")

derek.setBrightness(255)
derek.transitionTime(10)
derek.LEDstartHueA(0)
derek.LEDstartHueB(120)
derek.LEDendHue(270)
derek.LEDstartHueA(90)

derek.servo1speed(0)
derek.servo2speed(180)
derek.transitionType("RETURN")