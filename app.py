from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS 
import json
import os

app = Flask(__name__, static_folder='static')
CORS(app) 

DATA_FILE = 'sensor_data.json' 


def load_sensor_data():
    if os.path.exists(DATA_FILE):
        with open(DATA_FILE, 'r') as f:
            try:
                return json.load(f)
            except json.JSONDecodeError:
                return {} 
    return {} 

def save_sensor_data(data):
    with open(DATA_FILE, 'w') as f:
        json.dump(data, f, indent=4) 

@app.route('/')
def serve_index():
    return send_from_directory(app.static_folder, 'index.html')

@app.route('/<path:path>')
def serve_static(path):
    return send_from_directory(app.static_folder, path)

@app.route('/update_sensor', methods=['POST'])
def update_sensor_data():
    if request.is_json:
        sensor_data = request.get_json()
        print("Received data:", sensor_data) 


        all_data = load_sensor_data()

        device_id = sensor_data.get('device_id', 'unknown_device')
        all_data[device_id] = sensor_data 

        save_sensor_data(all_data) 

        return jsonify({"message": "Data received and saved", "status": "success"}), 200
    return jsonify({"message": "Request must be JSON", "status": "error"}), 400

@app.route('/get_sensor_data', methods=['GET'])
def get_sensor_data():
    data = load_sensor_data()
    return jsonify(data), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)