#include "cmsisnn.h"
#include <stdint.h>
#include <string.h>

#include "model_weights.h"
#include "model_meta.h"

#define K_H     3
#define K_W     3
#define IN_CH   2
#define OUT_CH  32
#define FC1_IN  (OUT_CH * 3 * 3)
#define FC1_OUT 64
#define FC2_OUT 9

static uint8_t pick_first_empty_by_order(const uint8_t board[9])
{
    static const uint8_t order[9] = {4, 0, 2, 6, 8, 1, 3, 5, 7};
    for (int i = 0; i < 9; ++i) {
        if (board[order[i]] == 0) return order[i];
    }
    return 0;
}

int cmsisnn_initialize(void)
{
    return 0;
}

uint8_t cmsisnn_best_move(uint8_t board[9])
{
#if !defined(CONV_W_SHAPE) || !defined(FC2_W_SHAPE)
    return pick_first_empty_by_order(board);
#endif

    /* ---------- 量化参数 ---------- */
#ifdef CONV_W_SCALE
    const float conv_w_scale = (CONV_W_SCALE > 1e-6f) ? CONV_W_SCALE : 1.0f;
#else
    const float conv_w_scale = 1.0f;
#endif
#ifdef CONV_W_ZERO_POINT
    const int conv_w_zp = CONV_W_ZERO_POINT;
#else
    const int conv_w_zp = 0;
#endif
#ifdef CONV_B_SCALE
    const float conv_b_scale = (CONV_B_SCALE > 1e-6f) ? CONV_B_SCALE : 1.0f;
#else
    const float conv_b_scale = input_scale * conv_w_scale;
#endif

#ifdef FC1_W_SCALE
    const float fc1_w_scale = (FC1_W_SCALE > 1e-6f) ? FC1_W_SCALE : 1.0f;
#else
    const float fc1_w_scale = 1.0f;
#endif
#ifdef FC1_W_ZERO_POINT
    const int fc1_w_zp = FC1_W_ZERO_POINT;
#else
    const int fc1_w_zp = 0;
#endif
#ifdef FC1_B_SCALE
    const float fc1_b_scale = (FC1_B_SCALE > 1e-6f) ? FC1_B_SCALE : 1.0f;
#else
    const float fc1_b_scale = input_scale * fc1_w_scale;
#endif

#ifdef FC2_W_SCALE
    const float fc2_w_scale = (FC2_W_SCALE > 1e-6f) ? FC2_W_SCALE : 1.0f;
#else
    const float fc2_w_scale = 1.0f;
#endif
#ifdef FC2_W_ZERO_POINT
    const int fc2_w_zp = FC2_W_ZERO_POINT;
#else
    const int fc2_w_zp = 0;
#endif
#ifdef FC2_B_SCALE
    const float fc2_b_scale = (FC2_B_SCALE > 1e-6f) ? FC2_B_SCALE : 1.0f;
#else
    const float fc2_b_scale = input_scale * fc2_w_scale;
#endif

    /* ---------- 输入层：当前玩家视角 ---------- */
    float input_f[3][3][IN_CH];
    for (int pos = 0; pos < 9; ++pos) {
        int h = pos / 3;
        int w = pos % 3;
        input_f[h][w][0] = (board[pos] == 2) ? 1.0f : 0.0f;  /* X = 当前玩家(AI) */
        input_f[h][w][1] = (board[pos] == 1) ? 1.0f : 0.0f;  /* O = 对手 */
    }

    /* ---------- Conv2D + ReLU ---------- */
    /* 权重格式: [out, H, W, in] = [32, 3, 3, 2] */
    float conv_out[3][3][OUT_CH];
    memset(conv_out, 0, sizeof(conv_out));

    for (int oh = 0; oh < 3; ++oh) {
        for (int ow = 0; ow < 3; ++ow) {
            for (int oc = 0; oc < OUT_CH; ++oc) {
                float acc = ((float)conv_b[oc]) * conv_b_scale;
                for (int kh = 0; kh < K_H; ++kh) {
                    for (int kw = 0; kw < K_W; ++kw) {
                        int ih = oh + kh - 1;
                        int iw = ow + kw - 1;
                        if (ih < 0 || ih >= 3 || iw < 0 || iw >= 3) continue;
                        for (int ic = 0; ic < IN_CH; ++ic) {
                            int widx = ((oc * K_H + kh) * K_W + kw) * IN_CH + ic;
                            float wval = ((float)((int8_t)conv_w[widx] - conv_w_zp)) * conv_w_scale;
                            acc += input_f[ih][iw][ic] * wval;
                        }
                    }
                }
                if (acc < 0.0f) acc = 0.0f;
                conv_out[oh][ow][oc] = acc;
            }
        }
    }

    /* ---------- Flatten ---------- */
    float fc1_in[FC1_IN];
    int fidx = 0;
    for (int h = 0; h < 3; ++h)
    for (int w = 0; w < 3; ++w)
    for (int c = 0; c < OUT_CH; ++c)
        fc1_in[fidx++] = conv_out[h][w][c];

    /* ---------- FC1 (288 -> 64) + ReLU ---------- */
    /* 权重格式: [out, in] = [64, 288] */
    float fc1_out[FC1_OUT];
    for (int o = 0; o < FC1_OUT; ++o) {
        float acc = ((float)fc1_b[o]) * fc1_b_scale;
        for (int i = 0; i < FC1_IN; ++i) {
            int widx = o * FC1_IN + i;
            float wval = ((float)((int8_t)fc1_w[widx] - fc1_w_zp)) * fc1_w_scale;
            acc += fc1_in[i] * wval;
        }
        if (acc < 0.0f) acc = 0.0f;
        fc1_out[o] = acc;
    }

    /* ---------- FC2 (64 -> 9) ---------- */
    /* 权重格式: [out, in] = [9, 64] */
    float fc2_out[FC2_OUT];
    for (int o = 0; o < FC2_OUT; ++o) {
        float acc = ((float)fc2_b[o]) * fc2_b_scale;
        for (int i = 0; i < FC1_OUT; ++i) {
            int widx = o * FC1_OUT + i;
            float wval = ((float)((int8_t)fc2_w[widx] - fc2_w_zp)) * fc2_w_scale;
            acc += fc1_out[i] * wval;
        }
        fc2_out[o] = acc;
    }

    /* ---------- 选最大且空的位置 ---------- */
    int best_pos = -1;
    float best_val = -1e9f;
    for (int pos = 0; pos < 9; ++pos) {
        if (board[pos] != 0) continue;
        if (fc2_out[pos] > best_val) {
            best_val = fc2_out[pos];
            best_pos = pos;
        }
    }

    return (best_pos < 0) ? pick_first_empty_by_order(board) : (uint8_t)best_pos;
}
