"""Flask application for controlling Skutter robots.

Requires Flask, SkutterZero.
Run via `python3 skuttercontrol.py`."""

from flask import Flask
from flask import redirect
from flask import render_template
from flask import url_for
from flask import request
from flask import flash
# skutterzero - and its config file `skutter_mac.py` are copied over from the main
# directory here. Which means there's a fragmentation risk. Ugh.
from skutterzero import Skutter
from itertools import islice # Used to skip the first form field in POST handler

app = Flask(__name__)
app.secret_key = '2B4C0s8ObsIuL6pxvbfJaTm+MJcfvLKSw9IzTNlr1T5pYJZ1kSzQz'

# Instantiate Skutters:
derek = Skutter("D07")
daphne = Skutter("D08")
hettie = Skutter("D04")

def str_to_class(s):
    """Used for converting a form-passed string into the name of a Skutter object.
    We check if the named Skutter exists, which should help avoid arbitrary code
    execution from a web form (<ulp>).
    TODO: This isn't actually tested. It does what's wanted when passed sane data, but
          behaviour with bad data is unknown. Hopefully the app blows up."""
    if s in globals() and isinstance(globals()[s], Skutter):
        return globals()[s]
    return None


@app.route("/")
def home():
    return render_template("home.html")


# It's ridiculous to handle each skutter separately here. This needs fixing.
@app.route("/derek")
def renderDerek():
    print(derek.servo1positionA)
    return render_template("derek.html", servo1positionA=derek.servo1positionA)
    # The above throws a bound method exception, because servo1positionA is both an instance variable and a method. Oops.

@app.route("/daphne")
def renderDaphne():
    return render_template("daphne.html")

@app.route("/hettie")
def renderHettie():
    return render_template("hettie.html")
    # return render_template("hettie.html", servo1positionA=hettie.servo1positionA,
    #                                       servo1positionB=hettie.servo1positionB,
    #                                       servo2positionA=hettie.servo2positionA,
    #                                       servo2positionB=hettie.servo2positionB,
    #                                       LEDstartHueA=hettie.LEDstartHueA,
    #                                       LEDstartHueA=hettie.LEDstartHueB,
    #                                       LEDendHueA=hettie.LEDendHueA,
    #                                       LEDendHueB=hettie.LEDendHueB,
    #                                       transitionTime=hettie.transitionTime)


# One MQTT form handler to rule them all.
@app.route("/send", methods=["POST"])
def send():
    """Handle form submission and fire off MQTT messages."""
    skutterName = request.form.get("skutterName")
    flash_message = "Skutter: " + skutterName
    for v in islice(request.form, 1, None): # Skip the first entry, that's the name of the skutter
        flash_message += " | %s : %s " %(v, request.form[v])
        # Now call method v on object skutterName, passing in the form value request.form[v]
        # This relies on SkutterZero to handle string/hex data passed in. Ouch.
        getattr(str_to_class(skutterName), v)(request.form[v])


    # TODO: Handle the individual elements of the form submission, rather than hard-coding here.
    # servo1speed = request.form.get("servo1speed")
    # knobData1 = request.form.get("knob-data-1")
    # flash_message = "Skutter: "+ skutterName + " | Servo1Speed: " + servo1speed + " | Knob data: " + knobData1
    # Work out which Skutter we're talking to from the form data, and command that one.
    
    # str_to_class(skutterName).servo1speed(servo1speed)
    # Give some mildly reassuring feedback to the user, that their data bas been acted upon.
    flash(flash_message)
    # Return us to the Skutter page from which we came.
    return redirect(skutterName)

if __name__ == '__main__':
    # This should probably be in the main body of the code, not tucked away down here.
    app.run(port=5000, debug=True)

