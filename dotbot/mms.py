import twilio.twiml
from flask import Flask, request
app = Flask(__name__)

@app.route('/twilio', methods=['POST'])
def twilio_endpoint():
    r = twilio.twiml.Response()
    r.message(str(request.form))
    return str(r)

if __name__ == '__main__':
    app.run(debug=True)
