from flask import Flask,request, send_from_directory
import serial
import time

def get_serial():
	global ard
	ard = serial.Serial("COM3", 9600)

def setColor(r,g,b):
	ard.write(endcode_data(21,r,g,b))

def endcode_data(*args):
	return b"".join([chr(el) for el in args])

def get_color(hex_color_string):
	if not hex_color_string:
		return 127, 127, 127
	color_as_num = int(hex_color_string, 16)
	return color_as_num >> 16, (color_as_num >> 8) & 0xff, color_as_num & 0xff

flag = 0

def get_flag():
	return flag

def set_flag(x):
	flag = x

app = Flask(__name__)

@app.route("/")
def  index():
	if get_flag() == 0:
		get_serial()
		set_flag(1)
	return send_from_directory('static', 'index.html')

@app.route("/set", methods=["POST"])
def set():
	if request.method == 'POST':
		red, green, blue = get_color(request.form.get("color")))
		setColor(red, green, blue)
	return "done", 200

@app.route("/music", methods=["POST"])
def music():
	if request.method == 'POST':
		ard.write(endcode_data(1))
	return "done", 200

@app.route("/rainbow", methods=["POST"])
def rainbow():
	ard.write(endcode_data(41,20))
	return "done", 200

@app.route("/strobe", methods=["POST"])
def strobe():
	red, green, blue = get_color(request.form.get("color"))
	speed = request.form.get('speed', 14)
	ard.write(endcode_data(61, red, green, blue, int(speed)))
	return "done", 200

@app.route("/theaterChase", methods=["POST"])
def theaterChase():
	red, green, blue = get_color(request.form.get("color")))
	delay = request.form.get('delay', 0)
	ard.write(endcode_data(62, red, green, blue, int(delay)))
	return "done", 200

@app.route("/breathing", methods=["POST"])
def breathing():
	red, green, blue = get_color(request.form.get("color"))
	delay = request.form.get('delay', 20)
	ard.write(endcode_data(63, red, green, blue, int(delay)))
	return "done", 200

@app.route("/wave", methods=["POST"])
def wave():
	red, green, blue = get_color(request.form.get("color"))
	delay = request.form.get('delay', 30)
	width = request.form.get('width', 0)
	ard.write(endcode_data(64, red, green, blue, int(delay), int(width)))
	return "done", 200

@app.route('/js/<path:path>')
def send_js(path):
    return send_from_directory('static/js', path)

@app.route('/css/<path:path>')
def send_css(path):
    return send_from_directory('static/css', path)

if __name__ == "__main__":
	app.run(debug=True, host="0.0.0.0")

