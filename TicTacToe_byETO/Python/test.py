import onnx
import torch
import numpy as np
import tensorflow as tf
from onnx_tf.backend import prepare

print("NumPy:", np.__version__)
print("PyTorch:", torch.__version__)
print("TensorFlow:", tf.__version__)
print("ONNX:", onnx.__version__)
print("All imports OK")