import serial
import socket
def get_serial():
	global ard
	ard = serial.Serial("COM3", 9600)


from flask import Flask,request, send_from_directory, g, render_template
import time

def write_data(data):
	#ard.write(data)
	clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	clientsocket.connect(('localhost', 8888))
	clientsocket.send(data)

def setColor(r,g,b):
	write_data(endcode_data(21,r,g,b))

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
	return send_from_directory('static', 'index.html')

@app.route("/connectSerial", methods=["POST","GET"])
def connectSerial():
	g.ard = serial.Serial("COM3", 9600)
	return "done", 200

@app.route("/solidColor", methods=["POST","GET"])
def set():
	if request.method == 'POST':
		red, green, blue = get_color(request.form.get("color"))
		setColor(red, green, blue)
		return "done", 200
	elif request.method == 'GET':
		colors = [{"name":"color"}]
		return render_template("control.html", target="solidColor", colors=colors)

@app.route("/bigPixels", methods=["POST","GET"])
def bigPixels():
	if request.method == 'POST':
		red, green, blue = get_color(request.form.get("color"))
		write_data(endcode_data(22,red,green,blue))
		return "done", 200
	elif request.method == 'GET':
		colors = [{"name":"color"}]
		return render_template("control.html", target="bigPixels", colors=colors)

@app.route("/music", methods=["POST","GET"])
def music():
	if request.method == 'POST':
		write_data(endcode_data(1))
		return "done", 200
	elif request.method == 'GET':
		return render_template("control.html", target="music")

@app.route("/musicRainbow", methods=["POST","GET"])
def musicRainbow():
	if request.method == 'POST':
		write_data(endcode_data(2))
		return "done", 200
	elif request.method == 'GET':
		return render_template("control.html", target="musicRainbow")
		
@app.route("/fire", methods=["POST","GET"])
def fire():
	if request.method == 'POST':
		write_data(endcode_data(3))
		return "done", 200
	elif request.method == 'GET':
		return render_template("control.html", target="fire")

@app.route("/rainbow", methods=["POST","GET"])
def rainbow():
	if request.method == 'POST':
		delay = int(request.form.get('delay', 20))
		write_data(endcode_data(41, delay))
		return "done", 200
	elif request.method == 'GET':
		sliders = [{"name": "delay", "min": 1, "max": 100, "default":20}]
		return render_template("control.html", target="rainbow", sliders=sliders)

@app.route("/rainbowCycle", methods=["POST","GET"])
def rainbowCycle():
	if request.method == 'POST':
		delay = int(request.form.get('delay', 20))
		write_data(endcode_data(42, delay))
		return "done", 200
	elif request.method == 'GET':
		sliders = [{"name": "delay", "min": 1, "max": 100, "default":20}]
		return render_template("control.html", target="rainbowCycle", sliders=sliders)

@app.route("/theaterChaseRainbow", methods=["POST","GET"])
def theaterChaseRainbow():
	if request.method == 'POST':
		delay = int(request.form.get('delay', 20))
		write_data(endcode_data(43, delay))
		return "done", 200
	elif request.method == 'GET':
		sliders = [{"name": "delay", "min": 1, "max": 100, "default":20}]
		return render_template("control.html", target="theaterChaseRainbow", sliders=sliders)

@app.route("/rainbowBreathing", methods=["POST","GET"])
def rainbowBreathing():
	if request.method == 'POST':
		delay = int(request.form.get('delay', 20))
		write_data(endcode_data(44, delay))
		return "done", 200
	elif request.method == 'GET':
		sliders = [{"name": "delay", "min": 1, "max": 100, "default":20}]
		return render_template("control.html", target="rainbowBreathing", sliders=sliders)

@app.route("/strobe", methods=["POST","GET"])
def strobe():
	if request.method == 'POST':
		red, green, blue = get_color(request.form.get("color"))
		speed = request.form.get('speed', 14)
		write_data(endcode_data(61, red, green, blue, int(speed)))
		return "done", 200
	elif request.method == 'GET':
		colors = [{"name":"color"}]
		sliders = [{"name": "speed", "min": 1, "max": 100, "default":14}]
		return render_template("control.html", target="strobe", colors=colors, sliders=sliders)

@app.route("/theaterChase", methods=["POST","GET"])
def theaterChase():
	if request.method == 'POST':
		red, green, blue = get_color(request.form.get("color"))
		delay = request.form.get('delay', 10)
		write_data(endcode_data(62, red, green, blue, int(delay)))
		return "done", 200
	elif request.method == 'GET':
		colors = [{"name":"color"}]
		sliders = [{"name": "delay", "min": 1, "max": 100, "default":20}]
		return render_template("control.html", target="theaterChase", colors=colors, sliders=sliders)

@app.route("/breathing", methods=["POST","GET"])
def breathing():
	if request.method == 'POST':
		red, green, blue = get_color(request.form.get("color"))
		delay = request.form.get('delay', 20)
		write_data(endcode_data(63, red, green, blue, int(delay)))
		return "done", 200
	elif request.method == 'GET':
		colors = [{"name":"color"}]
		sliders = [{"name": "delay", "min": 1, "max": 100, "default":20}]
		return render_template("control.html", target="breathing", colors=colors, sliders=sliders)

@app.route("/wave", methods=["POST","GET"])
def wave():
	if request.method == 'POST':
		red, green, blue = get_color(request.form.get("color"))
		delay = request.form.get('delay', 30)
		width = request.form.get('width', 0)
		write_data(endcode_data(65, red, green, blue, int(delay), int(width)))
		return "done", 200
	elif request.method == 'GET':
		colors = [{"name":"color"}]
		sliders = [{"name": "delay", "min": 1, "max": 100, "default":30}, {"name": "width", "min": 1, "max": 64, "default":10}]
		return render_template("control.html", target="wave", colors=colors, sliders=sliders)

@app.route("/dither", methods=["POST","GET"])
def dither():
	if request.method == 'POST':
		red, green, blue = get_color(request.form.get("color"))
		delay = request.form.get('delay', 30)
		write_data(endcode_data(64, red, green, blue, int(delay)))
		return "done", 200
	elif request.method == 'GET':
		colors = [{"name":"color"}]
		sliders = [{"name": "delay", "min": 1, "max": 100, "default":30}]
		return render_template("control.html", target="dither", colors=colors, sliders=sliders)

@app.route("/threeZone", methods=["POST","GET"])
def threeZone():
	if request.method == 'POST':
		red1, green1, blue1 = get_color(request.form.get("color 1"))
		red2, green2, blue2 = get_color(request.form.get("color 2"))
		red3, green3, blue3 = get_color(request.form.get("color 3"))
		write_data(endcode_data(81, red1, green1, blue1, red2, green2, blue2, red3, green3, blue3))
		return "done", 200
	elif request.method == 'GET':
		colors = [{"name":"color 1"}, {"name":"color 2"}, {"name":"color 3"}]
		return render_template("control.html", target="threeZone", colors=colors)

@app.route('/js/<path:path>')
def send_js(path):
    return send_from_directory('static/js', path)

@app.route('/css/<path:path>')
def send_css(path):
    return send_from_directory('static/css', path)

if __name__ == "__main__":
	##get_serial()
	app.run(debug=True, host="0.0.0.0")

