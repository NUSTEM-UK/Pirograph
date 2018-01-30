
"""Testing (in the ad-hoc sense).

Note the import form: `import skutterzero` doesn't do what we intend.
"""

from skutterzero import Skutter

MY_SKUTTER = Skutter("D10")

MY_MAC = MY_SKUTTER.getMac()
print(MY_MAC)

# MY_SKUTTER.TestSend()

MY_SKUTTER.setBrightness(50)
MY_SKUTTER.LEDstartColour(0.0)
