
"""Testing (in the ad-hoc sense).

Note the import form: `import skutterzero` doesn't do what we intend.
"""

from skutterzero import Skutter
from time import sleep

boris = Skutter("D07")

# MY_MAC = boris.getMac()
# print(MY_MAC)

boris.setBrightness(0)
boris.LEDcolour(90)
boris.setBrightness(100)
sleep(0.5)
boris.LEDcolour(120)
boris.setBrightness(255)

delay = 1

boris.servoSpeed(90)
boris.servo2speed(90)
sleep(delay)

boris.servoSpeed(120)
sleep(delay)
boris.servoSpeed(90.0)
sleep(delay)
boris.servoSpeed(75)
sleep(delay)
boris.servoSpeed(90)
sleep(delay)

boris.LEDcolour(0)
sleep(delay)
boris.setBrightness(50)
boris.LEDcolour(120)

boris.servoSpeed(0.0)
sleep(delay)
boris.servoSpeed(90.0)
sleep(delay)
boris.servo2speed(0.0)
sleep(delay)
boris.servo2speed(180.0)

# for x in range(25):
#     boris.setBrightness(x * 10)
#     sleep(0.2)

