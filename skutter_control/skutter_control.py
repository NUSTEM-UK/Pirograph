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

app = Flask(__name__)
app.secret_key = '2B4C0s8ObsIuL6pxvbfJaTm+MJcfvLKSw9IzTNlr1T5pYJZ1kSzQz'

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
def derek():
    return render_template("derek.html")

@app.route("/daphne")
def daphne():
    return render_template("daphne.html")

# One MQTT form handler to rule them all.
@app.route("/send", methods=["POST"])
def send():
    """Handle form submission and fire off MQTT messages.
    Currently, works only with a single form field. There's probably a neat way of:
    - stepping through form fields
    - firing off those methods to the appropriate skutter
    - not blowing up when it all goes wrong
    """
    skutterName = request.form.get("skutterName")
    servo1speed = request.form.get("servo1speed")
    knobData1 = request.form.get("knob-data-1")
    flash_message = "Skutter: "+ skutterName + " | Servo1Speed: " + servo1speed + " | Knob data: " + knobData1
    # Work out which Skutter we're talking to from the form data, and command that one.
    str_to_class(skutterName).servo1speed(servo1speed)
    # Give some mildly reassuring feedback to the user, that their data bas been acted upon.
    flash(flash_message)
    # Return us to the Skutter page from which we came.
    return redirect(url_for(skutterName))

if __name__ == '__main__':
    # Instantiate the Skutter objects here, so they're available app-wide.
    # This should probably be in the main body of the code, not tucked away down here.
    derek = Skutter("D07")
    daphne = Skutter("D08")
    app.run(port=5000, debug=True)

