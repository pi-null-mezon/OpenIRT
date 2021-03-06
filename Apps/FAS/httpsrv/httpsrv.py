# ------------------------------------------
# Classification http server
#
# (C) 2020 Alex A. Taranov, Moscow, Russia
# email pi-null-mezon@yandex.ru
# ------------------------------------------

import subprocess
import os
import sys
import uuid
import flask
import json
import socket
import struct
from werkzeug.utils import secure_filename
from waitress import serve

OS_WIN = False
if sys.platform == 'win32':
    OS_WIN = True

# Specify if oictcli should be used. If False then python tcp socket will be used
use_oictcli = False
# Specify address where srv is listening
srvaddr = os.getenv("CSRV_ADDR","127.0.0.1")
# Specify port where srv is listining
srvport = int(os.getenv("CSRV_PORT", 8080))
# Specify API routing prefix
apiprefix = "/fas"
# Specify where files should be uploaded
UPLOAD_FOLDER = '.'
if OS_WIN:
    UPLOAD_FOLDER = 'C:\Testdata'
# Specify allowed files extensions
ALLOWED_EXTENSIONS = set(['png', 'jpg', 'jpeg'])
# Flask's stuff
app = flask.Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER


# http://flask.pocoo.org/docs/0.12/patterns/fileuploads/
def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS


def randomize_name(filename):
    return str(uuid.uuid4()) + '.' + filename.rsplit('.', 1)[1].lower()


@app.route("%s/status" % apiprefix, methods=['GET'])
def get_status():
    return flask.jsonify({'status': 'Success', 'version': '1.5.0.0'}), 200


@app.route("%s/classify" % apiprefix, methods=['POST'])
def classify():
    if 'file' not in flask.request.files:
        return flask.jsonify({"status": "Error", "info": "Part 'file' is missing in request"}), 400
    file = flask.request.files['file']
    if file.filename == '':
        return flask.jsonify({"status": "Error", "info": "Empty filename"}), 400
    if file and allowed_file(file.filename):
        if use_oictcli:
            filename = randomize_name(secure_filename(file.filename))
            filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            file.save(filepath)
            oictsrvoutput = subprocess.check_output(
                ["oictcli", "-a%s" % srvaddr, "-p%d" % srvport, "-i%s" % filepath, "-t1", "-d"])
        else:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((srvaddr, srvport))
            try:
                content = file.read()
                sock.send(struct.pack('!Bi', 1, len(content)))
                sock.send(content)
                reply = b''
                while len(reply) < 4:
                    reply += sock.recv(4)
                length = struct.unpack('!i', reply[:4])[0]
                oictsrvoutput = b''
                while len(oictsrvoutput) < length:
                    oictsrvoutput += sock.recv(512)
            finally:
                sock.close()
        if OS_WIN:
            oictsrvoutput = oictsrvoutput.decode('cp1251')
        oictsrvjson = json.loads(oictsrvoutput)
        response = flask.make_response(oictsrvjson, 200 if oictsrvjson['status'].lower() == 'success' else 400)
        return response
    return flask.jsonify({"status": "Error", "info": "Invalid file extension, allowed %s" % ','.join(
        str(s) for s in ALLOWED_EXTENSIONS)}), 400


@app.route("%s/predict" % apiprefix, methods=['POST'])
def predict():
    if 'file' not in flask.request.files:
        return flask.jsonify({"status": "Error", "info": "Part 'file' is missing in request"}), 400
    file = flask.request.files['file']
    if file.filename == '':
        return flask.jsonify({"status": "Error", "info": "Empty filename"}), 400
    if file and allowed_file(file.filename):
        if use_oictcli:
            filename = randomize_name(secure_filename(file.filename))
            filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            file.save(filepath)
            oictsrvoutput = subprocess.check_output(
                ["oictcli", "-a%s" % srvaddr, "-p%d" % srvport, "-i%s" % filepath, "-t2", "-d"])
        else:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((srvaddr, srvport))
            try:
                content = file.read()
                sock.send(struct.pack('!Bi', 2, len(content)))
                sock.send(content)
                reply = b''
                while len(reply) < 4:
                    reply += sock.recv(4)
                length = struct.unpack('!i', reply[:4])[0]
                oictsrvoutput = b''
                while len(oictsrvoutput) < length:
                    oictsrvoutput += sock.recv(512)
            finally:
                sock.close()
        if OS_WIN:
            oictsrvoutput = oictsrvoutput.decode('cp1251')
        oictsrvjson = json.loads(oictsrvoutput)
        response = flask.make_response(oictsrvjson, 200 if oictsrvjson['status'].lower() == 'success' else 400)
        return response
    return flask.jsonify({"status": "Error", "info": "Invalid file extension, allowed %s" % ','.join(
        str(s) for s in ALLOWED_EXTENSIONS)}), 400


@app.route("%s/labels" % apiprefix, methods=['GET'])
def labels():
    if use_oictcli:
        oictsrvoutput = subprocess.check_output(["oictcli", "-a%s" % srvaddr, "-p%d" % srvport, "-t3"])
    else:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((srvaddr, srvport))
        try:
            sock.send(struct.pack('!B', 3))
            reply = b''
            while len(reply) < 4:
                reply += sock.recv(4)
            length = struct.unpack('!i', reply[:4])[0]
            oictsrvoutput = b''
            while len(oictsrvoutput) < length:
                oictsrvoutput += sock.recv(512)
        finally:
            sock.close()
    if OS_WIN:
        oictsrvoutput = oictsrvoutput.decode('cp1251')
    response = flask.make_response(oictsrvoutput, 200)
    response.headers['Content-Type'] = "application/json"
    return response


if __name__ == "__main__":
    serve(app, host="0.0.0.0", port=5000)
