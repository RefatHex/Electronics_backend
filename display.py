import os
import requests
from flask import Flask, render_template, send_file

app = Flask(__name__)

API_URL = "https://refathex.pythonanywhere.com"

@app.route('/')
def index():
    response = requests.get(f"{API_URL}/get_entry")
    data = response.json()["latest_entry"]

    ph_value = data["ph_value"]
    temperature = data["temperature"]
    flow_value = data["flow_value"]
    turbidity = data["turbidity"]

    return render_template('index.html', ph_value=ph_value, temperature=temperature,
                           flow_value=flow_value, turbidity=turbidity)

@app.route('/logs')
def logs():
    response = requests.get(f"{API_URL}/get_entry")
    data = response.json()["latest_entry"]

    ph_value = data["ph_value"]
    temperature = data["temperature"]
    flow_value = data["flow_value"]
    turbidity = data["turbidity"]

    logs = []

    # pH Value analysis
    if float(ph_value) > 12:
        logs.append("As the pH is over 12, it is considered alkaline. You need to add a neutralizing solution.")
    elif float(ph_value) < 6:
        logs.append("As the pH is below 6, it is considered acidic. You need to add a base solution.")
    else:
        logs.append("pH is within acceptable range.")

    # Temperature analysis
    temp_val = float(temperature)
    if temp_val > 35:
        logs.append(f"Temperature is {temperature}°C, which is too high. Cooling actions may be required.")
    elif temp_val < 10:
        logs.append(f"Temperature is {temperature}°C, which is too low. Heating actions may be required.")
    else:
        logs.append("Temperature is within acceptable range.")

    # Flow rate analysis
    flow_val = float(flow_value)
    if flow_val > 50:
        logs.append(f"Flow rate is {flow_value} L/s, which is high. Consider reducing the flow.")
    elif flow_val < 5:
        logs.append(f"Flow rate is {flow_value} L/s, which is too low. Consider increasing the flow.")
    else:
        logs.append("Flow rate is within acceptable range.")

    # Turbidity analysis
    turbidity_val = float(turbidity)
    if turbidity_val > -1:
        logs.append(f"Turbidity is {turbidity}, which is too high. Filtration may be required.")
    elif turbidity_val < 5:
        logs.append(f"Turbidity is {turbidity}, which is too low. Check for possible issues.")
    else:
        logs.append("Turbidity is within acceptable range.")

    return render_template('logs.html', logs=logs)

@app.route('/get_database')
def download_database():
    FILE_DOWNLOAD_URL = "https://refathex.pythonanywhere.com/get_database"
    
    response = requests.get(FILE_DOWNLOAD_URL)
    file_path = "downloaded_database.xlsx"  

    with open(file_path, 'wb') as file:
        file.write(response.content)

    return send_file(file_path, as_attachment=True)

if __name__ == '__main__':
    app.run(debug=True, port=8082)
