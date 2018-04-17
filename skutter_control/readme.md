# Skutter Control flask app

Install Python packages listed in [requirements.txt](requirements.txt), via `pip3 install ...`, then run the development server with `python3 skutter_control.py`. Interface will load on `127.0.0.1:5000` (NB. localhost-only).

Incidentally, `mosquitto_sub -h [hostIP] -v -t pirograph/+` will output MQTT broker action on the `pirograph/#` topics. We've previously written our own Python for that, which keeps breaking between Python2 & 3. Whoops. (On Pi, needs `sudo apt install mosquitto-clients`)

## Next steps

In no particular order:

- Find and integrate some nice Bootstrap controls.
- Decide on a layout and implement it.
- Form validation, though that's probably in the neat controls.
- Work out how to serve Skutter control panels uniquely. This probably needs a browser cookie since we're effectively tracking sessions, if not users, and hence maybe a database on the back-end.
- Actually wire up the skutter controls.
- How does the user know which skutter they're controlling? May need to theme our Bootstrap layouts, using something like [PaintStrap](http://paintstrap.com).
- It'd be lovely to be able to define individual skutters via some sort of YAML-like grammar, and have that parsed into the layout and MQTT. Then we could define new skutters on the fly without to delve beyond configuration files.
- Input sanity checking. I think this is what WTForms is for.