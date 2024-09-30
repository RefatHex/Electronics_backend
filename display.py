from flask import Flask, render_template
import requests

app = Flask(__name__)

API_URL = "https://refathex.pythonanywhere.com/get_entry"

@app.route('/')
def index():
    response = requests.get(API_URL)
    data = response.json()["latest_entry"]

    ph_value = data["ph_value"]
    temperature = data["temperature"].replace("C", "")
    flow_value = data["flow_value"]
    turbidity = data["turbidity"]

    return render_template('index.html', ph_value=ph_value, temperature=temperature,
                           flow_value=flow_value, turbidity=turbidity)

@app.route('/logs')
def logs():
    # Fetch data from the API
    response = requests.get(API_URL)
    data = response.json()["latest_entry"]

    ph_value = data["ph_value"]
    temperature = data["temperature"].replace("C", "")
    flow_value = data["flow_value"]
    turbidity = data["turbidity"]

    # Prepare logs based on the data
    logs = []

    # pH Value analysis
    if float(ph_value) > 12:
        logs.append("As the pH is over 12, it is considered alkaline. You need to add a neutralizing solution.")
        print("pH is too high, adding log.")
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
    print(logs)

    return render_template('logs.html', logs=logs)


if __name__ == '__main__':
    app.run(debug=True, port=8082)
