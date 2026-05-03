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
#define OUTPUT_SCALE 0.7870457768440247f
#define OUTPUT_ZERO_POINT 31

// conv_w
#define CONV_W_SHAPE {32, 3, 3, 2}
#define CONV_W_DTYPE "int8"
#define CONV_W_SCALE_COUNT 32
#define CONV_W_SCALE 0.011534756136825308f
#define CONV_W_ZERO_POINT 0

// conv_b
#define CONV_B_SHAPE {32}
#define CONV_B_DTYPE "int32"
#define CONV_B_SCALE_COUNT 32
#define CONV_B_SCALE 4.523434040493157e-05f
#define CONV_B_ZERO_POINT 0

// fc1_w
#define FC1_W_SHAPE {64, 288}
#define FC1_W_DTYPE "int8"
#define FC1_W_SCALE 0.02637648396193981f
#define FC1_W_ZERO_POINT 0

// fc1_b
#define FC1_B_SHAPE {64}
#define FC1_B_DTYPE "int32"
#define FC1_B_SCALE 0.0004439380718395114f
#define FC1_B_ZERO_POINT 0

// fc2_w
#define FC2_W_SHAPE {9, 64}
#define FC2_W_DTYPE "int8"
#define FC2_W_SCALE 0.018075697124004364f
#define FC2_W_ZERO_POINT 0

// fc2_b
#define FC2_B_SHAPE {9}
#define FC2_B_DTYPE "int32"
#define FC2_B_SCALE 0.0019305653404444456f
#define FC2_B_ZERO_POINT 0

#endif // MODEL_META_H
