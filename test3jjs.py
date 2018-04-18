from skutterzero import Skutter

derek = Skutter("D07")

# derek.setBrightness(255)
# derek.transitionTime(10)
# derek.transitionType("RETURN")

# derek.LEDstartHueA(110)
# derek.LEDstartHueB(180)

# derek.servo1speedA(0)
# derek.servo1speedB(180)
# derek.servo2speedA(180)
# derek.servo2speedB(90)

def str_to_class(s):
    """Used for converting a form-passed string into the name of a Skutter object.
    We check if the named Skutter exists, which should help avoid arbitrary code
    execution from a web form (<ulp>).
    TODO: This isn't actually tested. It does what's wanted when passed sane data, but
          behaviour with bad data is unknown. Hopefully the app blows up."""
    if s in globals() and isinstance(globals()[s], Skutter):
        return globals()[s]
    return None

print(derek.LEDcount)

derek.servo1positionA

skutterName = "derek"
methodToCall = "getMac"

print( str_to_class(skutterName) )
print( getattr( str_to_class(skutterName), methodToCall)() )
print(derek.getMac())

derek.servo1position = 120
