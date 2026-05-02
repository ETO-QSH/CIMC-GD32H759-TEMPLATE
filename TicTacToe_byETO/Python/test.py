import tensorflow as tf
interpreter = tf.lite.Interpreter(model_path="output/tictactoe_int8.tflite")
interpreter.allocate_tensors()
d = interpreter.get_input_details()[0]
print(f"scale: {d['quantization_parameters']['scales'][0]}")
print(f"zero_point: {d['quantization_parameters']['zero_points'][0]}")
