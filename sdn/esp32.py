import tensorflow as tf
import numpy as np

# Load TFLite model
interpreter = tf.lite.Interpreter(model_path="sensor_model.tflite")
interpreter.allocate_tensors()

# Dummy sensor data
input_data = np.array([[25.0, 7.5, 12.0, 1.0, 0]])  # [temperature, pH, turbidity, flow, trash_detected]

input_index = interpreter.get_input_details()[0]["index"]
output_details = interpreter.get_output_details()

interpreter.set_tensor(input_index, input_data.astype(np.float32))
interpreter.invoke()

outputs = [interpreter.get_tensor(o["index"]) for o in output_details]
print(outputs)
