#include "tflm.h"

#include <stdint.h>
#include <cstring>

// TFLite Micro headers
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_allocator.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/core/api/error_reporter.h"
#include "tensorflow/lite/core/api/flatbuffer_conversions.h"

// Model data: defined in Model/model_data.h
extern "C" {
    extern const unsigned char model_data[];
    extern const unsigned int model_len;
}

// ============================================================================
// Global TFLM State
// ============================================================================

namespace {
    // Arena buffer for TFLite Micro interpreter (10 KB for this model)
    // Adjust size based on profiling; typical ranges 8-16 KB for small models
    static const size_t kArenaSize = 10 * 1024;  // 10 KB
    static uint8_t g_arena_buffer[kArenaSize];

    // Interpreter instance (created during initialization)
    static tflite::MicroInterpreter* g_interpreter = nullptr;

    // Resolver for operator registration
    static tflite::MicroMutableOpResolver<6>* g_resolver = nullptr;

    // Model and input/output tensor pointers
    static const tflite::Model* g_model = nullptr;
    static TfLiteTensor* g_input_tensor = nullptr;
    static TfLiteTensor* g_output_tensor = nullptr;

    // Error reporting callback
    class SimpleErrorReporter : public tflite::ErrorReporter {
    public:
        int Report(const char* format, va_list args) override {
            // Silent for now; replace with logging if needed
            (void)format;
            (void)args;
            return 0;
        }
    };

    static SimpleErrorReporter g_error_reporter;
}

// ============================================================================
// Helper: Fallback move selection (order-based)
// ============================================================================

static uint8_t pick_first_empty_by_order(const uint8_t board[9])
{
    static const uint8_t order[9] = {4, 0, 2, 6, 8, 1, 3, 5, 7};
    for (uint8_t i = 0; i < 9; ++i)
    {
        uint8_t index = order[i];
        if (board[index] == 0U)
        {
            return index;
        }
    }
    return 0U;
}

// ============================================================================
// TFLM Initialization
// ============================================================================

int ai_tflm_initialize(void)
{
    // Load the model
    g_model = tflite::GetModel(model_data);
    if (g_model->version() != TFLITE_SCHEMA_VERSION) {
        return -1;  // Version mismatch
    }

    // Create resolver and register required operators
    static tflite::MicroMutableOpResolver<6> resolver;
    g_resolver = &resolver;

    // Register operators needed by the model
    if (g_resolver->AddConv2D() != kTfLiteOk) return -2;
    if (g_resolver->AddRelu() != kTfLiteOk) return -3;
    if (g_resolver->AddFullyConnected() != kTfLiteOk) return -4;
    if (g_resolver->AddReshape() != kTfLiteOk) return -5;
    if (g_resolver->AddQuantize() != kTfLiteOk) return -6;
    if (g_resolver->AddDequantize() != kTfLiteOk) return -7;

    // Create the interpreter
    static tflite::MicroInterpreter interpreter(
        g_model,
        *g_resolver,
        g_arena_buffer,
        kArenaSize,
        &g_error_reporter);

    g_interpreter = &interpreter;

    // Allocate tensors
    if (g_interpreter->AllocateTensors() != kTfLiteOk) {
        return -8;
    }

    // Get input and output tensors
    g_input_tensor = g_interpreter->input(0);
    if (g_input_tensor == nullptr) {
        return -9;
    }

    g_output_tensor = g_interpreter->output(0);
    if (g_output_tensor == nullptr) {
        return -10;
    }

    // Validate tensor shapes
    // Input: expected to be [1, 3, 3, 2] for batch=1, H=3, W=3, C=2
    // Output: expected to be [1, 9] for batch=1, 9 logits
    if (g_input_tensor->dims->size != 4) {
        return -11;
    }
    if (g_output_tensor->dims->size != 2) {
        return -12;
    }

    return 0;  // Success
}

// ============================================================================
// TFLM Inference
// ============================================================================

uint8_t ai_tflm_best_move(uint8_t board[9])
{
    // Validation check
    if (g_interpreter == nullptr || g_input_tensor == nullptr || 
        g_output_tensor == nullptr) {
        // TFLM not initialized, fallback to heuristic
        return pick_first_empty_by_order(board);
    }

    // ========================================================================
    // Convert board[9] to 2-channel input tensor
    // Channel 0: X positions (1 if player X, 0 otherwise)
    // Channel 1: O positions (1 if player O, 0 otherwise)
    //
    // Input tensor layout (NHWC): [batch=1, height=3, width=3, channels=2]
    // ========================================================================

    // Determine input data type
    TfLiteType input_type = g_input_tensor->type;
    
    if (input_type == kTfLiteFloat32) {
        // Input is float32
        float* input_data = g_interpreter->typed_input_tensor<float>(0);
        if (input_data == nullptr) {
            return pick_first_empty_by_order(board);
        }

        // Populate the 2 channels from board
        // Layout: input_data[h * 3 * 2 + w * 2 + c] for position (h, w, c)
        for (int pos = 0; pos < 9; ++pos) {
            // Map board index to (h, w) in 3x3 grid
            int h = pos / 3;
            int w = pos % 3;
            
            // Channel 0: X marker (1.0 if player X, 0.0 otherwise)
            input_data[h * 3 * 2 + w * 2 + 0] = (board[pos] == 1) ? 1.0f : 0.0f;
            
            // Channel 1: O marker (1.0 if player O, 0.0 otherwise)
            input_data[h * 3 * 2 + w * 2 + 1] = (board[pos] == 2) ? 1.0f : 0.0f;
        }
    }
    else if (input_type == kTfLiteInt8) {
        // Input is int8 (quantized)
        int8_t* input_data = g_interpreter->typed_input_tensor<int8_t>(0);
        if (input_data == nullptr) {
            return pick_first_empty_by_order(board);
        }

        // Quantization parameters (typically zero_point=-128, scale=1.0 for input)
        // Simplified: assume binary representation (0 or 1 in original scale)
        for (int pos = 0; pos < 9; ++pos) {
            int h = pos / 3;
            int w = pos % 3;
            
            // Channel 0: X marker
            input_data[h * 3 * 2 + w * 2 + 0] = (board[pos] == 1) ? 127 : -128;
            
            // Channel 1: O marker
            input_data[h * 3 * 2 + w * 2 + 1] = (board[pos] == 2) ? 127 : -128;
        }
    }
    else {
        // Unsupported input type
        return pick_first_empty_by_order(board);
    }

    // ========================================================================
    // Run inference
    // ========================================================================
    if (g_interpreter->Invoke() != kTfLiteOk) {
        // Inference failed, fallback to heuristic
        return pick_first_empty_by_order(board);
    }

    // ========================================================================
    // Parse output logits and select best valid move
    // Output: 9 logits corresponding to board positions 0-8
    // ========================================================================

    // Get output data
    TfLiteType output_type = g_output_tensor->type;
    float best_logit = -1e9f;
    uint8_t best_move = 0;

    if (output_type == kTfLiteFloat32) {
        float* output_data = g_interpreter->typed_output_tensor<float>(0);
        if (output_data == nullptr) {
            return pick_first_empty_by_order(board);
        }

        // Find the position with highest logit that is empty
        for (int pos = 0; pos < 9; ++pos) {
            if (board[pos] == 0 && output_data[pos] > best_logit) {
                best_logit = output_data[pos];
                best_move = pos;
            }
        }
    }
    else if (output_type == kTfLiteInt8) {
        // Output is int8 (quantized)
        int8_t* output_data = g_interpreter->typed_output_tensor<int8_t>(0);
        if (output_data == nullptr) {
            return pick_first_empty_by_order(board);
        }

        // Find the position with highest logit that is empty
        // Note: int8 range is -128 to 127
        int8_t best_logit_int = -128;
        for (int pos = 0; pos < 9; ++pos) {
            if (board[pos] == 0 && output_data[pos] > best_logit_int) {
                best_logit_int = output_data[pos];
                best_move = pos;
            }
        }
    }

    // If no empty position was found or no valid move selected, fallback
    if (best_move >= 9 || board[best_move] != 0) {
        return pick_first_empty_by_order(board);
    }

    return best_move;
}
