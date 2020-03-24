# -----------------------------------------------------------------
# Face Recognition http server
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
from waitress import serve
from werkzeug.utils import secure_filename

OS_WIN = False
if sys.platform == 'win32':
    OS_WIN = True

# Specify address where oirtsrv is listening
oirtsrvaddr = '127.0.0.1'
# Specify port where oirtsrv is listining
oirtsrvport = 8080
# Specify API routing prefix
apiprefix = '/iface'
# Specify where files should be stored on upload
UPLOAD_FOLDER = '/var/iface/local_storage'
if OS_WIN:
    UPLOAD_FOLDER = 'C:/Testdata/iFace/local_storage'
if not os.path.isdir(UPLOAD_FOLDER):
    os.mkdir(UPLOAD_FOLDER)
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
    return flask.jsonify({'status': 'Success', 'version': '1.0.0.0', 'datastorage': app.config['UPLOAD_FOLDER']}), 200


@app.route("%s/photo" % apiprefix, methods=['GET'])
def get_photo_v1():    
	labelinfo = base64.b64encode(flask.request.form['labelinfo'].encode('utf-8')).decode('utf-8')
	filenamelist = [f.rsplit('.', 1)[0] for f in os.listdir(app.config['UPLOAD_FOLDER']) if
					os.path.isfile(os.path.join(app.config['UPLOAD_FOLDER'], f))]
	if labelinfo in filenamelist:
		for extension in ALLOWED_EXTENSIONS:
			absfilename = os.path.join(app.config['UPLOAD_FOLDER'], f"{labelinfo}.{extension}")
			if os.path.exists(absfilename):
				return flask.send_from_directory(app.config['UPLOAD_FOLDER'], f"{labelinfo}.{extension}", as_attachment=True)
				#with open(absfilename, 'rb') as imgfile:
				   #return flask.jsonify({"status": "Success", "photo": base64.b64encode(imgfile.read()).decode('utf-8')}), 200
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
        oirtsrvoutput = subprocess.check_output(
            ["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-i%s" % filepath, "-l%s" % labelinfo, "-t1"])
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
	oirtsrvoutput = subprocess.check_output(
		["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-l%s" % labelinfo, "-t2"])
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
        filename = randomize_name(secure_filename(file.filename))
        filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
        file.save(filepath)
        oirtsrvoutput = subprocess.check_output(
            ["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-i%s" % filepath, "-d", "-t3"])
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
        filename = randomize_name(secure_filename(file.filename))
        filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
        file.save(filepath)
        oirtsrvoutput = subprocess.check_output(
            ["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-i%s" % filepath, "-d", "-t7"])
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
    oirtsrvoutput = subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-t4"])
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
        efilename = randomize_name(secure_filename(efile.filename))
        efilepath = os.path.join(app.config['UPLOAD_FOLDER'], efilename)
        efile.save(efilepath)
        vfilename = randomize_name(secure_filename(vfile.filename))
        vfilepath = os.path.join(app.config['UPLOAD_FOLDER'], vfilename)
        vfile.save(vfilepath)
        oirtsrvoutput = subprocess.check_output(
            ["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-i%s" % efilepath, "-v%s" % vfilepath, "-d",
             "-t5"])
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
    filepath = os.path.join(app.config['UPLOAD_FOLDER'], str(uuid.uuid4()) + '.json')
    with open(filepath, 'w') as outfile:
        json.dump(whitelist, outfile)
    oirtsrvoutput = subprocess.check_output(
        ["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-w%s" % filepath, "-d", "-t6"])
    if OS_WIN:
        oirtsrvoutput = oirtsrvoutput.decode('cp1251')
    oirtsrvjson = json.loads(oirtsrvoutput)
    response = flask.make_response(oirtsrvoutput, 200 if oirtsrvjson['status'].lower() == 'success' else 400)
    response.headers['Content-Type'] = "application/json"
    save_event('whitelist', oirtsrvjson)
    return response


@app.route("%s/whitelist/drop" % apiprefix, methods=['POST'])
def drop_whitelist():
    oirtsrvoutput = subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-t8"])
    if OS_WIN:
        oirtsrvoutput = oirtsrvoutput.decode('cp1251')
    response = flask.make_response(oirtsrvoutput, 200)
    response.headers['Content-Type'] = "application/json"
    save_event('whitelist/drop', json.loads(oirtsrvoutput))
    return response


if __name__ == "__main__":
    serve(app, host="0.0.0.0", port=5000)
