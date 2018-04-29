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
michael = Skutter("D11")
darcey = Skutter("D16")
jane = Skutter("D12")
margot = Skutter("D13")
rudolph = Skutter("D03")
marvel = Skutter("D04")

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

@app.route("/admin")
def renderAdmin():
    return render_template("admin.html")

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
    # return render_template("hettie.html")
    return render_template("hettie.html", servo1speedA=hettie.servo1speedA,
                                          servo1speedB=hettie.servo1speedB,
                                          servo2positionA=hettie.servo2positionA,
                                          servo2positionB=hettie.servo2positionB,
                                          LEDstartHueA=hettie.LEDstartHueA,
                                          LEDstartHueB=hettie.LEDstartHueB,
                                          LEDendHueA=hettie.LEDendHueA,
                                          LEDendHueB=hettie.LEDendHueB,
                                          transitionTime=hettie.transitionTime)


@app.route("/michael")
def renderMichael():
    return render_template("michael.html", stepper1speedA=michael.stepper1speedA,
                                           stepper1speedB=michael.stepper1speedB,
                                           stepper2angleA=michael.stepper2angleA,
                                           stepper2angleB=michael.stepper2angleB,
                                           LEDstartHueA=michael.LEDstartHueA,
                                           LEDstartHueB=michael.LEDstartHueB,
                                           LEDendHueA=michael.LEDendHueA,
                                           LEDendHueB=michael.LEDendHueB,
                                           transitionTime=michael.transitionTime)

@app.route("/darcey")
def renderDarcey():
    return render_template("darcey.html", stepper1speedA=darcey.stepper1speedA,
                                          stepper1speedB=darcey.stepper1speedB,
                                          stepper2angleA=darcey.stepper2angleA,
                                          stepper2angleB=darcey.stepper2angleB,
                                          LEDstartHueA=darcey.LEDstartHueA,
                                          LEDstartHueB=darcey.LEDstartHueB,
                                          LEDendHueA=darcey.LEDendHueA,
                                          LEDendHueB=darcey.LEDendHueB,
                                          transitionTime=darcey.transitionTime)

@app.route("/jane")
def renderJane():
    return render_template("jane.html", stepper1speedA=jane.stepper1speedA,
                                        stepper1speedB=jane.stepper1speedB,
                                        stepper2angleA=jane.stepper2angleA,
                                        stepper2angleB=jane.stepper2angleB,
                                        LEDstartHueA=jane.LEDstartHueA,
                                        LEDstartHueB=jane.LEDstartHueB,
                                        LEDendHueA=jane.LEDendHueA,
                                        LEDendHueB=jane.LEDendHueB,
                                        transitionTime=jane.transitionTime)

@app.route("/margot")
def renderMargot():
    return render_template("margot.html", stepper1speedA=margot.stepper1speedA,
                                          stepper1speedB=margot.stepper1speedB,
                                          stepper2angleA=margot.stepper2angleA,
                                          stepper2angleB=margot.stepper2angleB,
                                          LEDstartHueA=margot.LEDstartHueA,
                                          LEDstartHueB=margot.LEDstartHueB,
                                          LEDendHueA=margot.LEDendHueA,
                                          LEDendHueB=margot.LEDendHueB,
                                          transitionTime=margot.transitionTime)

@app.route("/rudolph")
def renderRudolph():
    return render_template("rudolph.html", stepper1speedA=rudolph.stepper1speedA,
                                           stepper1speedB=rudolph.stepper1speedB,
                                           LEDstartHueA=rudolph.LEDstartHueA,
                                           LEDstartHueB=rudolph.LEDstartHueB,
                                           LEDendHueA=rudolph.LEDendHueA,
                                           LEDendHueB=rudolph.LEDendHueB,
                                           transitionTime=rudolph.transitionTime)

@app.route("/marcel")
def renderMarcel():
    return render_template("marcel.html", stepper1speedA=marcel.stepper1speedA,
                                          stepper1speedB=marcel.stepper1speedB,
                                          LEDstartHueA=marcel.LEDstartHueA,
                                          LEDstartHueB=marcel.LEDstartHueB,
                                          LEDendHueA=marcel.LEDendHueA,
                                          LEDendHueB=marcel.LEDendHueB,
                                          transitionTime=marcel.transitionTime)

# One MQTT form handler to rule them all.
@app.route("/send", methods=["POST"])
def send():
    """Handle form submission and fire off MQTT messages."""
    print("<<< DEPLOY THE FORM HANDLER!")
    mySkutter = request.form.get("skutterName")
    flash_message = "Skutter: " + mySkutter
    for key in request.form:
        # Slip the skutterName, we already handled that
        if key =="skutterName":
            continue
        # output diagnostics back to the template
        flash_message += " | %s : %s " %(key, request.form[key])
        # output diagnostics to the console or error log
        print(key, request.form[key])
        # Now call method v on object skutterName, passing in the form value request.form[v]
        # This relies on SkutterZero to handle string/hex data passed in. Ouch.
        # Using setattr here to ensure we access the setter method named key
        setattr(str_to_class(mySkutter), key, request.form[key])
    print(">>> End of messages")

    # Set the template flash message and return to the corresponding form page
    # flash(flash_message)
    return redirect(mySkutter)

if __name__ == '__main__':
    # This should probably be in the main body of the code, not tucked away down here.
    app.run(port=5000, debug=True)

