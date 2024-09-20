from flask import Flask, request, jsonify
import openpyxl
import os

app = Flask(__name__)

def load_or_create_excel(file_name):
    if os.path.exists(file_name):
        workbook = openpyxl.load_workbook(file_name)
    else:
        workbook = openpyxl.Workbook()
        workbook.active.append(["timestamp", "Ph_Value", "Turbidity", "Temparature","Flow_Value"])  # Header
        workbook.save(file_name)
    return workbook

# API route to handle GET requests and save data to Excel
@app.route('/add_to_excel', methods=['GET'])
def add_to_excel():
    # Get query parameters from the request
    timestamp = request.args.get('timestamp')
    ph_value = request.args.get('ph_value')
    turbidity = request.args.get('turbidity')
    temparature = request.args.get('temparature')
    flow_value = request.args.get('flow_value')

    if not all([timestamp, ph_value, turbidity, temparature,flow_value]):
        return jsonify({"error": "Missing required parameters"}), 400

    # File name for the Excel file
    file_name = "data.xlsx"
    workbook = load_or_create_excel(file_name)
    sheet = workbook.active

    # Add the data to the Excel sheet
    sheet.append([timestamp, ph_value, turbidity, temparature,flow_value])
    workbook.save(file_name)

    return jsonify({"message": "Data added to Excel successfully!"}), 200

if __name__ == '__main__':
    app.run(debug=True)
