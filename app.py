from flask import Flask, request, jsonify
import openpyxl
import os
from datetime import datetime  # For generating timestamps

app = Flask(__name__)

def load_or_create_excel(file_name):
    if os.path.exists(file_name):
        workbook = openpyxl.load_workbook(file_name)
    else:
        workbook = openpyxl.Workbook()
        workbook.active.append(["Timestamp", "Ph_Value", "Turbidity", "Temperature", "Flow_Value"])  # Header
        workbook.save(file_name)
    return workbook

# API route to handle POST requests and save data to Excel
@app.route('/add_to_excel', methods=['POST'])
def add_to_excel():
    data = request.get_json()

    # Automatically generate the timestamp
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    ph_value = data.get('ph_value')
    turbidity = data.get('turbidity')
    temperature = data.get('temperature')
    flow_value = data.get('flow_value')

    if not all([ph_value, turbidity, temperature, flow_value]):
        return jsonify({"error": "Missing required parameters"}), 400

    # File name for the Excel file
    file_name = "data.xlsx"
    workbook = load_or_create_excel(file_name)
    sheet = workbook.active

    # Add the data to the Excel sheet
    sheet.append([timestamp, ph_value, turbidity, temperature, flow_value])
    workbook.save(file_name)

    return jsonify({"message": "Data added to Excel successfully!"}), 200

if __name__ == '__main__':
    app.run(debug=True)
