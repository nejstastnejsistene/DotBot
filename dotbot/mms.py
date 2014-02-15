from screenreader import read_game_screen
from ai import smart_path

import tempfile
import requests
import twilio.twiml
from flask import Flask, request
app = Flask(__name__)

@app.route('/twilio', methods=['POST'])
def twilio_endpoint():
    import sys
    sys.stdout = sys.stderr
    img = requests.get(request.form['MediaUrl0'])
    with open('screenshot.png', 'rw+') as f:
        f.write(img.raw.read())
    info = read_game_screen('screenshot.png')
    _, path = smart_path(info.colors)
    r = twilio.twiml.Response()
    r.message(str(path))
    return str(r)

if __name__ == '__main__':
    app.run(debug=True)
