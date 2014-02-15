from screenreader import read_game_screen
from ai import smart_path

import tempfile
import requests
import twilio.twiml
from flask import Flask, request
app = Flask(__name__)

@app.route('/twilio', methods=['POST'])
def twilio_endpoint():
    img = requests.get(request.form['MediaUrl0'])
    with open('screenshot.png') as f:
        f.write(img.text)
    info = read_game_screen('screenshot.png')
    _, path = smart_path(info)
    r = twilio.twiml.Response()
    r.message(str(path))
    return str(r)

if __name__ == '__main__':
    app.run(debug=True)
