from skutterzero import Skutter

derek = Skutter("D07")

derek.setBrightness(255)
derek.transitionTime(10)
derek.transitionType("RETURN")

derek.LEDstartHueA(110)
derek.LEDstartHueB(180)

derek.servo1speedA(0)
derek.servo1speedB(180)
derek.servo2speedA(180)
derek.servo2speedB(90)
