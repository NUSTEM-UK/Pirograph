
"""Testing (in the ad-hoc sense).

Note the import form: `import skutterzero` doesn't do what we intend.
"""

from skutterzero import Skutter
from time import sleep

MY_SKUTTER = Skutter("D10")

MY_MAC = MY_SKUTTER.getMac()
print(MY_MAC)

MY_SKUTTER.setBrightness(0)
MY_SKUTTER.LEDcolour(90)
MY_SKUTTER.setBrightness(100)
sleep(0.5)
MY_SKUTTER.LEDcolour(120)
MY_SKUTTER.setBrightness(255)

delay = 1
MY_SKUTTER.servoSpeed(0.0)
sleep(delay)
MY_SKUTTER.servoSpeed(90.0)
sleep(delay)
MY_SKUTTER.servo2speed(0.0)
sleep(delay)
MY_SKUTTER.servo2speed(180.0)

# for x in range(25):
#     MY_SKUTTER.setBrightness(x * 10)
#     sleep(0.2)
