import subprocess, os, sys, uuid, json
from flask import Flask, request, redirect, jsonify, make_response
from werkzeug.utils import secure_filename
from waitress import serve

OS_WIN = False
if sys.platform == 'win32':
	OS_WIN = True

# Specify address where srv is listening
srvaddr = "127.0.0.1"
# Specify port where srv is listining
srvport = 8080
# Specify API routing prefix
apiprefix = "/docrecservice/v1.0"
#Specify where files should be uploaded
UPLOAD_FOLDER = '.'
if OS_WIN:
	UPLOAD_FOLDER = 'C:\Testdata'
#Specify allowed files extensions	
ALLOWED_EXTENSIONS = set(['png', 'jpg', 'jpeg', 'bmp'])
# Flask's stuff
app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# http://flask.pocoo.org/docs/0.12/patterns/fileuploads/
def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS
		   
		   
def randomize_name(filename):
	return str(uuid.uuid4()) + '.' + filename.rsplit('.', 1)[1].lower() 
		   		   		   
@app.route("%s/classify" % apiprefix, methods=['POST'])
def classify():
	if 'file' not in request.files:
		return jsonify({"status": "Error", "info": "Part 'file' is missing in request"}), 400
	file = request.files['file']	
	if file.filename == '':
		return jsonify({"status": "Error", "info": "Empty filename"}), 400	
	if file and allowed_file(file.filename):
		filename = randomize_name(secure_filename(file.filename))
		filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename) 
		file.save(filepath)
		srvreply = subprocess.check_output(["oictcli", "-a%s" % srvaddr, "-p%d" % srvport,  "-i%s" % filepath, "-t1", "-d"]) 
		if OS_WIN:
			srvreply = srvreply.decode('cp1251')
		response = make_response(srvreply, 200)
		response.headers['Content-Type'] = "application/json"
		return response														
	return jsonify({"status": "Error", "info": "Invalid file extension, allowed %s" % ','.join(str(s) for s in ALLOWED_EXTENSIONS)}), 400
	
@app.route("%s/predict" % apiprefix, methods=['POST'])
def predict():
	if 'file' not in request.files:
		return jsonify({"status": "Error", "info": "Part 'file' is missing in request"}), 400
	file = request.files['file']	
	if file.filename == '':
		return jsonify({"status": "Error", "info": "Empty filename"}), 400	
	if file and allowed_file(file.filename):
		filename = randomize_name(secure_filename(file.filename))
		filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename) 
		file.save(filepath)
		srvreply = subprocess.check_output(["oictcli", "-a%s" % srvaddr, "-p%d" % srvport,  "-i%s" % filepath, "-t2", "-d"]) 
		if OS_WIN:
			srvreply = srvreply.decode('cp1251')
		response = make_response(srvreply, 200)
		response.headers['Content-Type'] = "application/json"
		return response														
	return jsonify({"status": "Error", "info": "Invalid file extension, allowed %s" % ','.join(str(s) for s in ALLOWED_EXTENSIONS)}), 400
	
@app.route("%s/labels" % apiprefix, methods=['GET'])
def labels():
		srvreply = subprocess.check_output(["oictcli", "-a%s" % srvaddr, "-p%d" % srvport,  "-t3"]) 
		if OS_WIN:
			srvreply = srvreply.decode('cp1251')
		response = make_response(srvreply, 200)
		response.headers['Content-Type'] = "application/json"
		return response														
	
if __name__ == "__main__":
	#app.run()
	serve(app, host = "0.0.0.0", port = 5000)
