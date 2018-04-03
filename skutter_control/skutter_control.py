"""Flask application for controlling Skutter robots.

Requires Flask, SkutterZero.
Run via `python3 skuttercontrol.py`."""

from flask import Flask
from flask import redirect
from flask import render_template
from flask import url_for
from flask import request
from skutterzero import Skutter

app = Flask(__name__)

@app.route("/")
def home():
    return render_template("home.html")

@app.route("/send", methods=["POST"])
def send():
    
    return

if __name__ == '__main__':
    derek = Skutter("D07")
    app.run(port=5000, debug=True)

