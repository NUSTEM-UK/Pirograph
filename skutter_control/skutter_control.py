"""Flask application for controlling Skutter robots.

Requires Flask, SkutterZero.
Run via `python3 skuttercontrol.py`."""

from flask import Flask
from flask import redirect
from flask import render_template
from flask import url_for
from flask import request
from flask import flash
from skutterzero import Skutter

app = Flask(__name__)
app.secret_key = '2B4C0s8ObsIuL6pxvbfJaTm+MJcfvLKSw9IzTNlr1T5pYJZ1kSzQz'

@app.route("/")
def home():
    return render_template("home.html")

@app.route("/derek")
def derek():
    return render_template("derek.html")

@app.route("/send", methods=["POST"])
def send():
    servo1speed = request.form.get("servo1speed")
    flash_message = "Servo1Speed: " + servo1speed
    derek.servo1speed(servo1speed)
    flash(flash_message)
    return redirect(url_for('derek'))

if __name__ == '__main__':
    derek = Skutter("D07")
    app.run(port=5000, debug=True)

