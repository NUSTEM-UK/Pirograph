
"""Testing (in the ad-hoc sense).

Note the import form: `import skutterzero` doesn't do what we intend.
"""

from skutterzero import Skutter

MY_SKUTTER = Skutter("D10")

MY_MAC = MY_SKUTTER.getMac()
print(MY_MAC)

my_dict = {"one": 1, "two": 2}
print(my_dict)
my_dict["one"] = 3
print(my_dict)

MY_SKUTTER.testSend()
