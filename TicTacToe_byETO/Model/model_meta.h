#ifndef MODEL_META_H
#define MODEL_META_H

// input
#define INPUT_SHAPE {1, 3, 3, 2}
#define INPUT_DTYPE "<class 'numpy.int8'>"
#define INPUT_SCALE 0.003921568859368563f
#define INPUT_ZERO_POINT -128

// output
#define OUTPUT_SHAPE {1, 9}
#define OUTPUT_DTYPE "<class 'numpy.int8'>"
#define OUTPUT_SCALE 0.46400853991508484f
#define OUTPUT_ZERO_POINT 62

// conv_w
#define CONV_W_SHAPE {32, 3, 3, 2}
#define CONV_W_DTYPE "int8"
#define CONV_W_SCALE 1.0f
#define CONV_W_ZERO_POINT 0

// conv_b
#define CONV_B_SHAPE {32}
#define CONV_B_DTYPE "int32"
#define CONV_B_SCALE 1.0f
#define CONV_B_ZERO_POINT 0

// fc1_w
#define FC1_W_SHAPE {64, 288}
#define FC1_W_DTYPE "int8"
#define FC1_W_SCALE 0.02753181755542755f
#define FC1_W_ZERO_POINT 0

// fc1_b
#define FC1_B_SHAPE {64}
#define FC1_B_DTYPE "int32"
#define FC1_B_SCALE 0.00041996428626589477f
#define FC1_B_ZERO_POINT 0

// fc2_w
#define FC2_W_SHAPE {9, 64}
#define FC2_W_DTYPE "int8"
#define FC2_W_SCALE 0.017019137740135193f
#define FC2_W_ZERO_POINT 0

// fc2_b
#define FC2_B_SHAPE {9}
#define FC2_B_DTYPE "int32"
#define FC2_B_SCALE 0.0011354975868016481f
#define FC2_B_ZERO_POINT 0

#endif // MODEL_META_H
