#include "cmsisnn.h"
#include <stdint.h>
#include <string.h>

#include "model_weights.h"
#include "model_meta.h"

#include "usart.h"

#define K_H     3
#define K_W     3
#define IN_CH   2
#define OUT_CH  32
#define FC1_IN  (OUT_CH * 3 * 3)
#define FC1_OUT 64
#define FC2_OUT 9

/* 外部声明 per-channel scale 数组（定义在 model_weights.h） */
extern const float conv_w_scale_per_channel[32];
extern const float conv_b_scale_per_channel[32];

static uint8_t pick_first_empty_by_order(const uint8_t board[9])
{
    static const uint8_t order[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
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
    const float input_scale = INPUT_SCALE;
    
    /* Conv: per-channel */
    const int conv_w_zp = CONV_W_ZERO_POINT;
    
    /* FC1: per-tensor */
    const float fc1_w_scale = FC1_W_SCALE;
    const int fc1_w_zp = FC1_W_ZERO_POINT;
    const float fc1_b_scale = input_scale * fc1_w_scale;
    
    /* FC2: per-tensor */
    const float fc2_w_scale = FC2_W_SCALE;
    const int fc2_w_zp = FC2_W_ZERO_POINT;
    const float fc2_b_scale = input_scale * fc2_w_scale;

    /* ---------- 输入层 ---------- */
    float input_f[3][3][IN_CH];
    for (int pos = 0; pos < 9; ++pos) {
        int h = pos / 3;
        int w = pos % 3;
        input_f[h][w][0] = (board[pos] == 2) ? 1.0f : 0.0f;
        input_f[h][w][1] = (board[pos] == 1) ? 1.0f : 0.0f;
    }
		
		/* ---------- 当前局势 ---------- */
    printf("input:\r\n");
    for (int h = 0; h < 3; h++) {
        for (int w = 0; w < 3; w++) {
            printf("[%.0f %.0f] ", input_f[h][w][0], input_f[h][w][1]);
        }
        printf("\r\n");
    }

    /* ---------- Conv2D + ReLU (per-channel) ---------- */
    float conv_out[3][3][OUT_CH];
    memset(conv_out, 0, sizeof(conv_out));

    for (int oh = 0; oh < 3; ++oh) {
        for (int ow = 0; ow < 3; ++ow) {
            for (int oc = 0; oc < OUT_CH; ++oc) {
                /* 关键：用 per-channel 的 bias scale */
                float acc = ((float)conv_b[oc]) * conv_b_scale_per_channel[oc];
                
                for (int kh = 0; kh < K_H; ++kh) {
                    for (int kw = 0; kw < K_W; ++kw) {
                        int ih = oh + kh - 1;
                        int iw = ow + kw - 1;
                        if (ih < 0 || ih >= 3 || iw < 0 || iw >= 3) continue;
                        for (int ic = 0; ic < IN_CH; ++ic) {
                            int widx = ((oc * K_H + kh) * K_W + kw) * IN_CH + ic;
                            float wval = ((float)((int8_t)conv_w[widx] - conv_w_zp)) 
                                         * conv_w_scale_per_channel[oc];
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

    /* ---------- FC1 + ReLU ---------- */
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

    /* ---------- FC2 ---------- */
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
		
		/* ---------- 模型输出 ---------- */
    printf("output:\r\n");
		for(int i = 0; i < FC2_OUT; i++){
				if(i % 3 == 0) printf("[");
				printf("%6.2f", fc2_out[i]);
				if((i + 1) % 3 == 0){
						printf("]\r\n");
				} else {
						printf(" ");
				}
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
