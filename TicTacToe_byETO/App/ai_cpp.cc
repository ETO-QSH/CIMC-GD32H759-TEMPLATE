// C++ TFLite Micro wrapper implementation
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_allocator.h"
#include "tensorflow/lite/core/c/common.h"
#include <cstdint>
#include <cstring>
#include <climits>

// ============================================================================
// Global State (static)
// ============================================================================

static tflite::MicroInterpreter* g_interpreter = nullptr;
static TfLiteTensor* g_input_tensor = nullptr;
static TfLiteTensor* g_output_tensor = nullptr;
static bool g_initialized = false;

// ============================================================================
// Helper: Register all required operators
// ============================================================================

namespace {

class OpResolver : public tflite::MicroMutableOpResolver<10> {
 public:
  OpResolver() {
    AddConv2D();
    AddRelu();
    AddRelu6();
    AddFullyConnected();
    AddReshape();
    AddQuantize();
    AddDequantize();
    AddPack();
    AddUnpack();
    AddLogistic();
  }
};

}  // namespace

// ============================================================================
// C++ Implementation Functions (called from C)
// ============================================================================

extern "C" {

int ai_cpp_initialize(const unsigned char* model_data, int model_len, 
                      uint8_t* arena, int arena_size)
{
    if (g_initialized)
        return 0;  // Already initialized

    // Get the model
    const tflite::Model* model = tflite::GetModel(model_data);
    if (!model) {
        return -1;  // Invalid model
    }

    // Create op resolver
    static OpResolver resolver;

    // Create interpreter
    static tflite::MicroInterpreter interpreter(
        model, resolver, arena, arena_size, nullptr);

    g_interpreter = &interpreter;

    // Allocate tensors
    if (g_interpreter->AllocateTensors() != kTfLiteOk) {
        return -2;  // Allocation failed
    }

    // Get input and output tensors
    g_input_tensor = g_interpreter->input(0);
    g_output_tensor = g_interpreter->output(0);

    if (!g_input_tensor || !g_output_tensor) {
        return -3;  // Invalid tensors
    }

    g_initialized = true;
    return 0;  // Success
}

uint8_t ai_cpp_best_move(uint8_t board[9])
{
    if (!g_initialized || !g_input_tensor || !g_output_tensor) {
        return 0;  // Default to first position if not initialized
    }

    // Prepare input data
    // Input format: 2 channels x 3x3 spatial (18 float values)
    // board[i]: 0 = empty, 1 = X (player), 2 = O (AI)
    
    if (g_input_tensor->type == kTfLiteFloat32) {
        float* input_data = g_input_tensor->data.f;
        
        // Channel 0: X positions (player)
        for (int i = 0; i < 9; i++) {
            input_data[i] = (board[i] == 1) ? 1.0f : 0.0f;
        }
        
        // Channel 1: O positions (AI)
        for (int i = 0; i < 9; i++) {
            input_data[9 + i] = (board[i] == 2) ? 1.0f : 0.0f;
        }
    }

    // Run inference
    TfLiteStatus status = g_interpreter->Invoke();
    if (status != kTfLiteOk) {
        return 0;  // Inference failed
    }

    // Parse output
    // Output: 9 logits (one per board position)
    // Find the position with highest score that is empty
    
    uint8_t best_move = 0;
    int best_score = INT_MIN;

    if (g_output_tensor->type == kTfLiteInt8) {
        const int8_t* output_data = g_output_tensor->data.int8;
        
        for (int i = 0; i < 9; i++) {
            if (board[i] == 0) {  // Only consider empty positions
                int score = output_data[i];
                if (score > best_score) {
                    best_score = score;
                    best_move = i;
                }
            }
        }
    } else if (g_output_tensor->type == kTfLiteFloat32) {
        const float* output_data = g_output_tensor->data.f;
        float best_float_score = -1e9f;
        
        for (int i = 0; i < 9; i++) {
            if (board[i] == 0) {  // Only consider empty positions
                if (output_data[i] > best_float_score) {
                    best_float_score = output_data[i];
                    best_move = i;
                }
            }
        }
    }

    return best_move;
}

}  // extern "C"
