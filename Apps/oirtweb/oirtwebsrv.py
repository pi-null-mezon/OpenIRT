import subprocess, os, uuid, platform
from flask import Flask, request, redirect, jsonify
from werkzeug.utils import secure_filename

# Specify address where oirtsrv is listening
oirtsrvaddr = "127.0.0.1"
# Specify port where oirtsrv is listining
oirtsrvport = 8080
# Specify API routing prefix
apiprefix = "/facerec/api/v1.0"
#Specify where files should be uploaded
UPLOAD_FOLDER = '/home/Testdata'
if platform == 'Windows':
	UPLOAD_FOLDER = 'C:\Testdata'
#Specify allowed files extensions	
ALLOWED_EXTENSIONS = set(['png', 'jpg', 'jpeg'])

# Flask's stuff
app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# http://flask.pocoo.org/docs/0.12/patterns/fileuploads/
def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS
		   
		   
def randomize_name(filename):
	return str(uuid.uuid4()) + '.' + filename.rsplit('.', 1)[1].lower() 
		   
		   		   
@app.route("%s/remember" % apiprefix, methods=['POST'])
def remember_face():
	if 'file' not in request.files:
		return jsonify({"status": "Error", "info": "file part is missing in request"}), 401
	file = request.files['file']	
	if file.filename == '':
		return jsonify({"status": "Error", "info": "Empty file name parameter"}), 402
	labelinfo = request.form['labelinfo']		
	if file and allowed_file(file.filename):
		filename = randomize_name(secure_filename(file.filename))
		filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename) 
		file.save(filepath)
		return subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%s" % str(oirtsrvport),
														"-i%s" % filepath,
														"-l%s" % labelinfo, "-d", "-t1"]), 200	
	return jsonify({"status": "Error", "info": "File you have try to upload seems to be bad"}), 403
	
	
@app.route("%s/delete" % apiprefix, methods=['POST'])
def delete_face():
	labelinfo = request.form['labelinfo']		
	return subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%s" % str(oirtsrvport), "-l%s" % labelinfo, "-t2"]), 200	

	
@app.route("%s/identify" % apiprefix, methods=['POST'])
def identify_face():
	if 'file' not in request.files:
		return jsonify({"status": "Error", "info": "file part is missing in request"}), 401
	file = request.files['file']	
	if file.filename == '':
		return jsonify({"status": "Error", "info": "Empty filename parameter"}), 402	
	if file and allowed_file(file.filename):
		filename = randomize_name(secure_filename(file.filename))
		filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename) 
		file.save(filepath)
		return subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%s" % str(oirtsrvport),
														"-i%s" % filepath, "-d", "-t3"]), 200	
	return jsonify({"status": "Error", "info": "File you have try to upload seems to be bad"}), 403
	
	
@app.route("%s/verify" % apiprefix, methods=['POST'])
def verify_face():
	if 'efile' not in request.files:
		return jsonify({"status": "Error", "info": "efile part is missing in request"}), 401
	efile = request.files['efile']	
	if efile.filename == '':
		return jsonify({"status": "Error", "info": "Empty efile name parameter"}), 402
	if 'vfile' not in request.files:
		return jsonify({"status": "Error", "info": "vfile part is missing in request"}), 402
	vfile = request.files['vfile']	
	if vfile.filename == '':
		return jsonify({"status": "Error", "info": "Empty vfile name parameter"}), 403			
	if efile and allowed_file(efile.filename) and vfile and allowed_file(vfile.filename):
		efilename = randomize_name(secure_filename(efile.filename))
		efilepath = os.path.join(app.config['UPLOAD_FOLDER'], efilename) 
		efile.save(efilepath)
		vfilename = randomize_name(secure_filename(vfile.filename))
		vfilepath = os.path.join(app.config['UPLOAD_FOLDER'], vfilename) 
		vfile.save(vfilepath)
		return subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%s" % str(oirtsrvport),
														"-i%s" % efilepath, "-v%s" % vfilepath, "-d", "-t5"]), 200	
	return jsonify({"status": "Error", "info": "Files you have try to upload seems to be bad"}), 405
	

@app.route("%s/labels" % apiprefix, methods=['GET'])
def get_labels():
	return subprocess.check_output(["oirtcli", "-a%s" % oirtsrvaddr, "-p%s" % str(oirtsrvport), "-t4"]), 200