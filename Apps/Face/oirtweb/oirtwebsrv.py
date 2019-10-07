import subprocess, os, sys, uuid, json, requests
from flask import Flask, request, redirect, jsonify, make_response
from werkzeug.utils import secure_filename
from waitress import serve

OS_WIN = False
if sys.platform == 'win32':
	OS_WIN = True

# Specify address where oirtsrv is listening
oirtsrvaddr = "127.0.0.1"
# Specify port where oirtsrv is listining
oirtsrvport = 8080
# Specify API routing prefix
apiprefix = "/facerec/api/v1.0"
#Specify where files should be uploaded
UPLOAD_FOLDER = '/home/Testdata'
if OS_WIN:
	UPLOAD_FOLDER = 'C:\Testdata'
#Specify allowed files extensions	
ALLOWED_EXTENSIONS = set(['png', 'jpg', 'jpeg'])
# Flask's stuff
app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# Eve server section to optionally post events into mongo storage
mongotoken = os.getenv('MONGO_AUTH_TOKEN') 
eveurl          = os.getenv('EVE_URL')
def save_response(methodname, response):
	if eveurl and mongotoken:
		requests.post(eveurl, json={'event': methodname, 'reply': response}, headers={'Authorization': 'Bearer ' + mongotoken})


# http://flask.pocoo.org/docs/0.12/patterns/fileuploads/
def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS
		   
		   
def randomize_name(filename):
	return str(uuid.uuid4()) + '.' + filename.rsplit('.', 1)[1].lower() 
		   
		   		   
@app.route("%s/remember" % apiprefix, methods=['POST'])
def remember_face():
	if 'file' not in request.files:
		return jsonify({"status": "Error", "info": "file part is missing in request"}), 400
	file = request.files['file']	
	if file.filename == '':
		return jsonify({"status": "Error", "info": "Empty file name parameter"}), 400
	labelinfo = request.form['labelinfo']	
	if file and allowed_file(file.filename):
		filename = randomize_name(secure_filename(file.filename))
		filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename) 
		file.save(filepath)
		oirtsrvoutput = subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-i%s" % filepath, "-l%s" % labelinfo, "-d", "-t1"])
		if OS_WIN:
			oirtsrvoutput = oirtsrvoutput.decode('cp1251')
		response = make_response(oirtsrvoutput, 200)
		response.headers['Content-Type'] = "application/json"
		save_response('remember',json.loads(oirtsrvoutput))
		return response													
	return jsonify({"status": "Error", "info": "File you have try to upload seems to be bad"}), 400
	
	
@app.route("%s/delete" % apiprefix, methods=['POST'])
def delete_face():
	labelinfo = request.form['labelinfo']
	oirtsrvoutput = subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-l%s" % labelinfo, "-t2"])
	if OS_WIN:
		oirtsrvoutput = oirtsrvoutput.decode('cp1251')	
	response = make_response(oirtsrvoutput, 200)
	response.headers['Content-Type'] = "application/json"
	save_response('delete',json.loads(oirtsrvoutput))
	return response	

	
@app.route("%s/identify" % apiprefix, methods=['POST'])
def identify_face():
	if 'file' not in request.files:
		return jsonify({"status": "Error", "info": "file part is missing in request"}), 400
	file = request.files['file']	
	if file.filename == '':
		return jsonify({"status": "Error", "info": "Empty filename parameter"}), 400	
	if file and allowed_file(file.filename):
		filename = randomize_name(secure_filename(file.filename))
		filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename) 
		file.save(filepath)
		oirtsrvoutput = subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport,  "-i%s" % filepath, "-d", "-t3"]) 
		if OS_WIN:
			oirtsrvoutput = oirtsrvoutput.decode('cp1251')
		response = make_response(oirtsrvoutput, 200)
		response.headers['Content-Type'] = "application/json"
		save_response('identify',json.loads(oirtsrvoutput))
		return response														
	return jsonify({"status": "Error", "info": "File you have try to upload seems to be bad"}), 400
	
	
@app.route("%s/recognize" % apiprefix, methods=['POST'])
def recognize_face():
	if 'file' not in request.files:
		return jsonify({"status": "Error", "info": "file part is missing in request"}), 400
	file = request.files['file']	
	if file.filename == '':
		return jsonify({"status": "Error", "info": "Empty filename parameter"}), 400	
	if file and allowed_file(file.filename):
		filename = randomize_name(secure_filename(file.filename))
		filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename) 
		file.save(filepath)
		oirtsrvoutput = subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport,  "-i%s" % filepath, "-d", "-t7"]) 
		if OS_WIN:
			oirtsrvoutput = oirtsrvoutput.decode('cp1251')
		response = make_response(oirtsrvoutput, 200)
		response.headers['Content-Type'] = "application/json"
		save_response('recognize',json.loads(oirtsrvoutput))
		return response														
	return jsonify({"status": "Error", "info": "File you have try to upload seems to be bad"}), 400
	
	
@app.route("%s/labels" % apiprefix, methods=['GET'])
def get_labels():
	oirtsrvoutput = subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-t4"])
	if OS_WIN:
			oirtsrvoutput = oirtsrvoutput.decode('cp1251')
	response = make_response(oirtsrvoutput, 200)
	response.headers['Content-Type'] = "application/json"
	save_response('labels',json.loads(oirtsrvoutput))
	return response
	
	
@app.route("%s/verify" % apiprefix, methods=['POST'])
def verify_face():
	if 'efile' not in request.files:
		return jsonify({"status": "Error", "info": "efile part is missing in request"}), 400
	efile = request.files['efile']	
	if efile.filename == '':
		return jsonify({"status": "Error", "info": "Empty efile name parameter"}), 400
	if 'vfile' not in request.files:
		return jsonify({"status": "Error", "info": "vfile part is missing in request"}), 400
	vfile = request.files['vfile']	
	if vfile.filename == '':
		return jsonify({"status": "Error", "info": "Empty vfile name parameter"}), 400			
	if efile and allowed_file(efile.filename) and vfile and allowed_file(vfile.filename):
		efilename = randomize_name(secure_filename(efile.filename))
		efilepath = os.path.join(app.config['UPLOAD_FOLDER'], efilename) 
		efile.save(efilepath)
		vfilename = randomize_name(secure_filename(vfile.filename))
		vfilepath = os.path.join(app.config['UPLOAD_FOLDER'], vfilename) 
		vfile.save(vfilepath)
		response = make_response(subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-i%s" % efilepath, "-v%s" % vfilepath, "-d", "-t5"]), 200)
		response.headers['Content-Type'] = "application/json"
		save_response('verify',json.loads(oirtsrvoutput))
		return response														
	return jsonify({"status": "Error", "info": "Files you have try to upload seems to be bad"}), 400	
	
	
@app.route("%s/whitelist" % apiprefix, methods=['POST'])
def set_whitelist():
	if not request.is_json: 
		return jsonify({"status": "Error", "info": "Your input is not JSON"}), 400 
	whitelist = request.get_json()
	filepath = os.path.join(app.config['UPLOAD_FOLDER'], str(uuid.uuid4()) + '.json') 
	with open(filepath, 'w') as outfile:
		json.dump(whitelist, outfile)
	response = make_response(subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-w%s" % filepath, "-d", "-t6"]), 200)
	response.headers['Content-Type'] = "application/json"
	save_response('whitelist',json.loads(oirtsrvoutput))
	return response

	
@app.route("%s/whitelist/drop" % apiprefix, methods=['POST'])
def drop_whitelist():
	response = make_response(subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%d" % oirtsrvport, "-t8"]), 200)
	response.headers['Content-Type'] = "application/json"
	save_response('whitelist/drop',json.loads(oirtsrvoutput))
	return response		
	
	
if __name__ == "__main__":
	#app.run()
	serve(app, host = "0.0.0.0", port = 5000)