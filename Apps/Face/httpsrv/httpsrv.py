# -----------------------------------------------------------------
# Face recognition http server
#
# (C) 2020 Alex A. Taranov, Moscow, Russia
# email a.a.taranov@nefrosovet.ru
# -----------------------------------------------------------------
import base64
import json
import os
import requests
import subprocess
import sys
import uuid
import flask
import socket
import struct
from waitress import serve
from werkzeug.utils import secure_filename

OS_WIN = False
if sys.platform == 'win32':
    OS_WIN = True

# Specify use oirtcli or not. If not, python tcp socket will be used
use_oirtcli = False
# Specify address where oirtsrv is listening
oirtsrvaddr = os.getenv('ISRV_ADDR', '127.0.0.1')
# Specify port where oirtsrv is listining
oirtsrvport = int(os.getenv('ISRV_PORT', 8080))
# Specify API routing prefix
apiprefix = '/iface'
# Specify where files should be stored on upload
UPLOAD_FOLDER = '/var/iface/local_storage'
if OS_WIN:
    UPLOAD_FOLDER = 'C:/Testdata/iFace/local_storage'
if not os.path.isdir(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER, exist_ok=True)
# Specify allowed files extensions
ALLOWED_EXTENSIONS = set(['png', 'jpg', 'jpeg'])
# Flask's stuff
app = flask.Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# Eve server section to post events into mongodb collection
mongotoken = os.getenv('MONGO_AUTH_TOKEN')
eveurl = os.getenv('EVE_URL')


def save_event(methodname, response):
    if eveurl and mongotoken:
        requests.post(eveurl, json={'event': methodname, 'reply': response},
                      headers={'Authorization': 'Bearer ' + mongotoken})


def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS


def randomize_name(filename):
    return str(uuid.uuid4()) + '.' + filename.rsplit('.', 1)[1].lower()


@app.route("%s/status" % apiprefix, methods=['GET'])
def get_status():
    return flask.jsonify({'status': 'Success', 'version': '1.5.0.0', 'datastorage': app.config['UPLOAD_FOLDER']}), 200


@app.route("%s/photo" % apiprefix, methods=['GET'])
def get_photo_v1():
    labelinfo = base64.b64encode(flask.request.form['labelinfo'].encode('utf-8')).decode('utf-8')
    filenamelist = [f.rsplit('.', 1)[0] for f in os.listdir(app.config['UPLOAD_FOLDER']) if
                    os.path.isfile(os.path.join(app.config['UPLOAD_FOLDER'], f))]
    if labelinfo in filenamelist:
        for extension in ALLOWED_EXTENSIONS:
            absfilename = os.path.join(app.config['UPLOAD_FOLDER'], f"{labelinfo}.{extension}")
            if os.path.exists(absfilename):
                return flask.send_from_directory(app.config['UPLOAD_FOLDER'], f"{labelinfo}.{extension}",
                                                 as_attachment=True)
            # with open(absfilename, 'rb') as imgfile:
            # return flask.jsonify({"status": "Success", "photo": base64.b64encode(imgfile.read()).decode('utf-8')}), 200
    return flask.jsonify({"status": "Error", "info": "No such labelinfo found or incorrect targetnum asked"}), 400


@app.route("%s/photo/<labelinfo>" % apiprefix, methods=['GET'])
def get_photo_v2(labelinfo):
    labelinfo = base64.b64encode(labelinfo.encode('utf-8')).decode('utf-8')
    filenamelist = [f.rsplit('.', 1)[0] for f in os.listdir(app.config['UPLOAD_FOLDER']) if
                    os.path.isfile(os.path.join(app.config['UPLOAD_FOLDER'], f))]
    if labelinfo in filenamelist:
        for extension in ALLOWED_EXTENSIONS:
            absfilename = os.path.join(app.config['UPLOAD_FOLDER'], f"{labelinfo}.{extension}")
            if os.path.exists(absfilename):
                return flask.send_from_directory(app.config['UPLOAD_FOLDER'], f"{labelinfo}.{extension}",
                                                 as_attachment=True)
    return flask.jsonify({"status": "Error", "info": "No such labelinfo found"}), 400


@app.route("%s/remember" % apiprefix, methods=['POST'])
def remember_face():
    if 'file' not in flask.request.files:
        return flask.jsonify({"status": "Error", "info": "file part is missing in request"}), 400
    file = flask.request.files['file']
    if file.filename == '':
        return flask.jsonify({"status": "Error", "info": "Empty file name parameter"}), 400
    labelinfo = flask.request.form['labelinfo']
    if file and allowed_file(file.filename):
        filepath = os.path.join(app.config['UPLOAD_FOLDER'],
                                str(base64.b64encode(labelinfo.encode('utf-8')).decode('utf-8')) + '.' +
                                file.filename.rsplit('.', 1)[1])
        file.save(filepath)
        if use_oirtcli:
            oirtsrvoutput = subprocess.check_output(
                ["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-i%s" % filepath, "-l%s" % labelinfo, "-t1"])
        else:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((oirtsrvaddr, oirtsrvport))
            try:
                data = labelinfo.encode('utf-8')
                sock.send(struct.pack('!Bi', 1, len(data)))
                sock.send(data)
                with open(filepath, 'rb') as file:
                    content = file.read()
                sock.send(struct.pack('!i', len(content)))
                sock.send(content)
                reply = b''
                while len(reply) < 4:
                    reply += sock.recv(4)
                length = struct.unpack('!i', reply[:4])[0]
                oirtsrvoutput = b''
                while len(oirtsrvoutput) < length:
                    oirtsrvoutput += sock.recv(512)
            finally:
                sock.close()
        if OS_WIN:
            oirtsrvoutput = oirtsrvoutput.decode('cp1251')
        oirtsrvjson = json.loads(oirtsrvoutput)
        template_created = oirtsrvjson['status'].lower() == 'success'
        response = flask.make_response(oirtsrvoutput, 200 if template_created else 400)
        response.headers['Content-Type'] = "application/json"
        save_event('remember', oirtsrvjson)
        if not template_created:
            os.remove(filepath)
        return response
    return flask.jsonify({"status": "Error", "info": "File you have try to upload seems to be bad"}), 400


@app.route("%s/delete" % apiprefix, methods=['DELETE'])
def delete_template():
    labelinfo = flask.request.form['labelinfo']
    if use_oirtcli:
        oirtsrvoutput = subprocess.check_output(
            ["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-l%s" % labelinfo, "-t2"])
    else:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((oirtsrvaddr, oirtsrvport))
        try:
            data = labelinfo.encode('utf-8')
            sock.send(struct.pack('!Bi', 2, len(data)))
            sock.send(data)
            reply = b''
            while len(reply) < 4:
                reply += sock.recv(4)
            length = struct.unpack('!i', reply[:4])[0]
            oirtsrvoutput = b''
            while len(oirtsrvoutput) < length:
                oirtsrvoutput += sock.recv(512)
        finally:
            sock.close()
    if OS_WIN:
        oirtsrvoutput = oirtsrvoutput.decode('cp1251')
    oirtsrvjson = json.loads(oirtsrvoutput)
    response = flask.make_response(oirtsrvoutput, 200 if oirtsrvjson['status'].lower() == 'success' else 400)
    response.headers['Content-Type'] = "application/json"
    save_event('delete', oirtsrvjson)
    # Remove photo from disk
    labelinfo = base64.b64encode(labelinfo.encode('utf-8')).decode('utf-8')
    for extension in ALLOWED_EXTENSIONS:
        absfilename = os.path.join(app.config['UPLOAD_FOLDER'], f"{labelinfo}.{extension}")
        if os.path.isfile(absfilename):
            os.remove(absfilename)
    return response


@app.route("%s/identify" % apiprefix, methods=['POST'])
def identify_face():
    if 'file' not in flask.request.files:
        return flask.jsonify({"status": "Error", "info": "file part is missing in request"}), 400
    file = flask.request.files['file']
    if file.filename == '':
        return flask.jsonify({"status": "Error", "info": "Empty filename parameter"}), 400
    if file and allowed_file(file.filename):
        if use_oirtcli:
            filename = randomize_name(secure_filename(file.filename))
            filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            file.save(filepath)
            oirtsrvoutput = subprocess.check_output(
                ["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-i%s" % filepath, "-d", "-t3"])
        else:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((oirtsrvaddr, oirtsrvport))
            try:
                content = file.read()
                sock.send(struct.pack('!Bi', 3, len(content)))
                sock.send(content)
                reply = b''
                while len(reply) < 4:
                    reply += sock.recv(4)
                length = struct.unpack('!i', reply[:4])[0]
                oirtsrvoutput = b''
                while len(oirtsrvoutput) < length:
                    oirtsrvoutput += sock.recv(512)
            finally:
                sock.close()
        if OS_WIN:
            oirtsrvoutput = oirtsrvoutput.decode('cp1251')
        oirtsrvjson = json.loads(oirtsrvoutput)
        response = flask.make_response(oirtsrvoutput, 200 if oirtsrvjson['status'].lower() == 'success' else 400)
        response.headers['Content-Type'] = "application/json"
        save_event('identify', oirtsrvjson)
        return response
    return flask.jsonify({"status": "Error", "info": "File you have try to upload seems to be bad"}), 400


@app.route("%s/recognize" % apiprefix, methods=['POST'])
def recognize_face():
    if 'file' not in flask.request.files:
        return flask.jsonify({"status": "Error", "info": "file part is missing in request"}), 400
    file = flask.request.files['file']
    if file.filename == '':
        return flask.jsonify({"status": "Error", "info": "Empty filename parameter"}), 400
    if file and allowed_file(file.filename):
        if use_oirtcli:
            filename = randomize_name(secure_filename(file.filename))
            filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            file.save(filepath)
            oirtsrvoutput = subprocess.check_output(
                ["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-i%s" % filepath, "-d", "-t7"])
        else:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((oirtsrvaddr, oirtsrvport))
            try:
                content = file.read()
                sock.send(struct.pack('!Bi', 7, len(content)))
                sock.send(content)
                reply = b''
                while len(reply) < 4:
                    reply += sock.recv(4)
                length = struct.unpack('!i', reply[:4])[0]
                oirtsrvoutput = b''
                while len(oirtsrvoutput) < length:
                    oirtsrvoutput += sock.recv(512)
            finally:
                sock.close()
        if OS_WIN:
            oirtsrvoutput = oirtsrvoutput.decode('cp1251')
        oirtsrvjson = json.loads(oirtsrvoutput)
        response = flask.make_response(oirtsrvoutput, 200 if oirtsrvjson['status'].lower() == 'success' else 400)
        response.headers['Content-Type'] = "application/json"
        save_event('recognize', oirtsrvjson)
        return response
    return flask.jsonify({"status": "Error", "info": "File you have try to upload seems to be bad"}), 400


@app.route("%s/labels" % apiprefix, methods=['GET'])
def get_labels():
    if use_oirtcli:
        oirtsrvoutput = subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-t4"])
    else:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((oirtsrvaddr, oirtsrvport))
        try:
            sock.send(struct.pack('!B', 4))
            reply = b''
            while len(reply) < 4:
                reply += sock.recv(4)
            length = struct.unpack('!i', reply[:4])[0]
            oirtsrvoutput = b''
            while len(oirtsrvoutput) < length:
                oirtsrvoutput += sock.recv(512)
        finally:
            sock.close()
    if OS_WIN:
        oirtsrvoutput = oirtsrvoutput.decode('cp1251')
    response = flask.make_response(oirtsrvoutput, 200)
    response.headers['Content-Type'] = "application/json"
    save_event('labels', json.loads(oirtsrvoutput))
    return response


@app.route("%s/verify" % apiprefix, methods=['POST'])
def verify_face():
    if 'efile' not in flask.request.files:
        return flask.jsonify({"status": "Error", "info": "efile part is missing in request"}), 400
    efile = flask.request.files['efile']
    if efile.filename == '':
        return flask.jsonify({"status": "Error", "info": "Empty efile name parameter"}), 400
    if 'vfile' not in flask.request.files:
        return flask.jsonify({"status": "Error", "info": "vfile part is missing in request"}), 400
    vfile = flask.request.files['vfile']
    if vfile.filename == '':
        return flask.jsonify({"status": "Error", "info": "Empty vfile name parameter"}), 400
    if efile and allowed_file(efile.filename) and vfile and allowed_file(vfile.filename):
        if use_oirtcli:
            efilename = randomize_name(secure_filename(efile.filename))
            efilepath = os.path.join(app.config['UPLOAD_FOLDER'], efilename)
            efile.save(efilepath)
            vfilename = randomize_name(secure_filename(vfile.filename))
            vfilepath = os.path.join(app.config['UPLOAD_FOLDER'], vfilename)
            vfile.save(vfilepath)
            oirtsrvoutput = subprocess.check_output(
                ["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-i%s" % efilepath, "-v%s" % vfilepath, "-d",
                 "-t5"])
        else:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((oirtsrvaddr, oirtsrvport))
            try:
                content = efile.read()
                sock.send(struct.pack('!Bi', 5, len(content)))
                sock.send(content)
                content = vfile.read()
                sock.send(struct.pack('!i', len(content)))
                sock.send(content)
                reply = b''
                while len(reply) < 4:
                    reply += sock.recv(4)
                length = struct.unpack('!i', reply[:4])[0]
                oirtsrvoutput = b''
                while len(oirtsrvoutput) < length:
                    oirtsrvoutput += sock.recv(512)
            finally:
                sock.close()
        if OS_WIN:
            oirtsrvoutput = oirtsrvoutput.decode('cp1251')
        oirtsrvjson = json.loads(oirtsrvoutput)
        response = flask.make_response(oirtsrvoutput, 200 if oirtsrvjson['status'].lower() == 'success' else 400)
        response.headers['Content-Type'] = "application/json"
        save_event('verify', oirtsrvjson)
        return response
    return flask.jsonify({"status": "Error", "info": "Files you have try to upload seems to be bad"}), 400


@app.route("%s/whitelist" % apiprefix, methods=['POST'])
def set_whitelist():
    if not flask.request.is_json:
        return flask.jsonify({"status": "Error", "info": "Your input is not JSON"}), 400
    whitelist = flask.request.get_json()
    if use_oirtcli:
        filepath = os.path.join(app.config['UPLOAD_FOLDER'], str(uuid.uuid4()) + '.json')
        with open(filepath, 'w') as outfile:
            json.dump(whitelist, outfile)
        oirtsrvoutput = subprocess.check_output(
            ["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-w%s" % filepath, "-d", "-t6"])
    else:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((oirtsrvaddr, oirtsrvport))
        try:
            data = str(whitelist).replace("'", "\"").encode('utf-8')
            sock.send(struct.pack('!Bi', 6, len(data)))
            sock.send(data)
            reply = b''
            while len(reply) < 4:
                reply += sock.recv(4)
            length = struct.unpack('!i', reply[:4])[0]
            oirtsrvoutput = b''
            while len(oirtsrvoutput) < length:
                oirtsrvoutput += sock.recv(512)
        finally:
            sock.close()
    if OS_WIN:
        oirtsrvoutput = oirtsrvoutput.decode('cp1251')
    oirtsrvjson = json.loads(oirtsrvoutput)
    response = flask.make_response(oirtsrvoutput, 200 if oirtsrvjson['status'].lower() == 'success' else 400)
    response.headers['Content-Type'] = "application/json"
    save_event('whitelist', oirtsrvjson)
    return response


@app.route("%s/whitelist/drop" % apiprefix, methods=['POST'])
def drop_whitelist():
    if use_oirtcli:
        oirtsrvoutput = subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-t8"])
    else:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((oirtsrvaddr, oirtsrvport))
        try:
            sock.send(struct.pack('!B', 8))
            reply = b''
            while len(reply) < 4:
                reply += sock.recv(4)
            length = struct.unpack('!i', reply[:4])[0]
            oirtsrvoutput = b''
            while len(oirtsrvoutput) < length:
                oirtsrvoutput += sock.recv(512)
        finally:
            sock.close()
    if OS_WIN:
        oirtsrvoutput = oirtsrvoutput.decode('cp1251')
    response = flask.make_response(oirtsrvoutput, 200)
    response.headers['Content-Type'] = "application/json"
    save_event('whitelist/drop', json.loads(oirtsrvoutput))
    return response


if __name__ == "__main__":
    serve(app, host="0.0.0.0", port=5000)
