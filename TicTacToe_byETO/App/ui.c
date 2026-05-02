#include "ui.h"
#include "OLED.h"

static void ui_draw_diamond(uint8_t x, uint8_t y, uint8_t radius);
static int8_t ui_abs_i8(int8_t value);
static void ui_clear_region(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
static void ui_draw_parallel_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t ox, int16_t oy);
static void ui_set_gram_point(uint8_t x, uint8_t y);
static void ui_draw_line_gram(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
static int16_t ui_clamp_i16(int16_t value, int16_t minValue, int16_t maxValue);
static void ui_get_win_line_points(uint8_t lineIndex, int16_t *x0, int16_t *y0, int16_t *x1, int16_t *y1, int16_t *dx, int16_t *dy, int16_t *px, int16_t *py, int16_t *yShift);

static void ui_draw_hline(uint8_t x1, uint8_t x2, uint8_t y)
{
    for (uint8_t x = x1; x <= x2; x++)
    {
        OLED_DrawPoint(x, y);
    }
}

static void ui_draw_vline(uint8_t x, uint8_t y1, uint8_t y2)
{
    for (uint8_t y = y1; y <= y2; y++)
    {
        OLED_DrawPoint(x, y);
    }
}

static void ui_draw_cross(uint8_t x, uint8_t y, uint8_t radius)
{
    for (uint8_t i = 0; i <= (uint8_t)(radius * 2); i++)
    {
        OLED_DrawPoint((uint8_t)(x - radius + i), (uint8_t)(y - radius + i));
        OLED_DrawPoint((uint8_t)(x - radius + i), (uint8_t)(y + radius - i));
    }
}

static void ui_draw_board(void)
{
    const uint8_t left = 40;
    const uint8_t top = 0;
    const uint8_t right = 86;
    const uint8_t bottom = 31;
    const uint8_t col1 = 55;
    const uint8_t col2 = 71;
    const uint8_t row1 = 10;
    const uint8_t row2 = 21;

    ui_draw_hline(left, right, top);
    ui_draw_hline(left, right, row1);
    ui_draw_hline(left, right, row2);
    ui_draw_hline(left, right, bottom);
    ui_draw_vline(left, top, bottom);
    ui_draw_vline(col1, top, bottom);
    ui_draw_vline(col2, top, bottom);
    ui_draw_vline(right, top, bottom);
}

static void ui_draw_side_info(uint8_t youScore, uint8_t aiScore)
{
    const uint8_t left_label_x = 8;
    const uint8_t right_label_x = 106;

    OLED_ShowString(left_label_x, 4, (uint8_t *)"YOU", 12);
    OLED_ShowNum((uint8_t)(left_label_x - 3), 18, youScore, 2, 12);
    OLED_ShowChar((uint8_t)(left_label_x + 15), 18, 'O', 12);

    OLED_ShowString(right_label_x, 4, (uint8_t *)"AI", 12);
    OLED_ShowNum((uint8_t)(right_label_x - 6), 18, aiScore, 2, 12);
    OLED_ShowChar((uint8_t)(right_label_x + 12), 18, 'X', 12);
}

void UI_ShowStaticScreen(void)
{
    OLED_Clear();
    ui_draw_side_info(0, 0);
    ui_draw_board();
    OLED_Refresh();
}

static void ui_cell_center(uint8_t cell, uint8_t *cx, uint8_t *cy)
{
    uint8_t col = cell % 3;
    uint8_t row = cell / 3;
    const uint8_t left = 40;
    const uint8_t col1 = 55;
    const uint8_t col2 = 71;
    const uint8_t right = 86;
    const uint8_t top = 0;
    const uint8_t row1 = 10;
    const uint8_t row2 = 21;
    const uint8_t bottom = 31;

    uint8_t xs[3] = { (uint8_t)((left + col1) / 2), (uint8_t)((col1 + col2) / 2), (uint8_t)((col2 + right) / 2) };
    uint8_t ys[3] = { (uint8_t)((top + row1) / 2), (uint8_t)((row1 + row2) / 2), (uint8_t)((row2 + bottom) / 2) };
    *cx = xs[col];
    *cy = ys[row];
}

void UI_DrawPieceAt(uint8_t cell, uint8_t piece)
{
    uint8_t x, y;
    ui_cell_center(cell, &x, &y);
    if (piece == 1)
    {
        ui_draw_diamond(x, y, 3);
    }
    else if (piece == 2)
    {
        ui_draw_cross(x, y, 3);
    }
}

static void ui_refresh_region(uint8_t x1, uint8_t x2, uint8_t page1, uint8_t page2)
{
    for (uint8_t p = page1; p <= page2; p++)
    {
        OLED_WR_Byte(0xb0 + p, OLED_CMD); // set page
        OLED_WR_Byte(((x1 & 0xf0) >> 4) | 0x10, OLED_CMD);
        OLED_WR_Byte((x1 & 0x0f), OLED_CMD);
        for (uint8_t x = x1; x <= x2; x++)
        {
            OLED_WR_Byte(OLED_GRAM[x][p], OLED_DATA);
        }
    }
}

static void ui_clear_region(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    if (x1 > x2 || y1 > y2)
    {
        return;
    }

    for (uint8_t x = x1; x <= x2; x++)
    {
        for (uint8_t y = y1; y <= y2; y++)
        {
            uint8_t page = (uint8_t)(y / 8);
            uint8_t bit = (uint8_t)(y % 8);
            OLED_GRAM[x][page] &= (uint8_t)(~(1U << bit));
        }
    }
}

void UI_DrawCursorDot(uint8_t cell, uint8_t visible, const uint8_t board[9])
{
    uint8_t x, y;
    ui_cell_center(cell, &x, &y);

    int16_t x1 = (int16_t)x - 1;
    int16_t x2 = (int16_t)x + 1;
    int16_t y1 = (int16_t)y - 1;
    int16_t y2 = (int16_t)y + 1;
    if (x1 < 0) x1 = 0;
    if (x2 > 127) x2 = 127;
    if (y1 < 0) y1 = 0;
    if (y2 > 31) y2 = 31;

    uint8_t page1 = (uint8_t)(y1 / 8);
    uint8_t page2 = (uint8_t)(y2 / 8);

    if (visible)
    {
        for (int16_t xi = x1; xi <= x2; xi++)
        {
            for (int16_t yi = y1; yi <= y2; yi++)
            {
                OLED_DrawPoint((uint8_t)xi, (uint8_t)yi);
            }
        }

        ui_refresh_region((uint8_t)x1, (uint8_t)x2, page1, page2);
    }
    else
    {
        ui_clear_region((uint8_t)x1, (uint8_t)y1, (uint8_t)x2, (uint8_t)y2);

        if (board[cell] != 0)
        {
            UI_DrawPieceAt(cell, board[cell]);

            if ((int16_t)(x - 3) < 0)
            {
                x1 = 0;
            }
            else
            {
                x1 = (int16_t)x - 3;
            }

            if ((int16_t)(x + 3) > 127)
            {
                x2 = 127;
            }
            else
            {
                x2 = (int16_t)x + 3;
            }

            if ((int16_t)(y - 3) < 0)
            {
                y1 = 0;
            }
            else
            {
                y1 = (int16_t)y - 3;
            }

            if ((int16_t)(y + 3) > 31)
            {
                y2 = 31;
            }
            else
            {
                y2 = (int16_t)y + 3;
            }
        }

        page1 = (uint8_t)(y1 / 8);
        page2 = (uint8_t)(y2 / 8);
        ui_refresh_region((uint8_t)x1, (uint8_t)x2, page1, page2);
    }
}

void UI_HideCursorNoRefresh(uint8_t cell, const uint8_t board[9])
{
    uint8_t x, y;
    ui_cell_center(cell, &x, &y);

    int16_t x1 = (int16_t)x - 1;
    int16_t x2 = (int16_t)x + 1;
    int16_t y1 = (int16_t)y - 1;
    int16_t y2 = (int16_t)y + 1;
    if (x1 < 0) x1 = 0;
    if (x2 > 127) x2 = 127;
    if (y1 < 0) y1 = 0;
    if (y2 > 31) y2 = 31;

    // clear cursor pixels from GRAM only
    ui_clear_region((uint8_t)x1, (uint8_t)y1, (uint8_t)x2, (uint8_t)y2);

    // if there's a piece in this cell, redraw it into GRAM
    if (board[cell] != 0)
    {
        UI_DrawPieceAt(cell, board[cell]);
    }

    // do NOT call ui_refresh_region here — caller will decide when to push GRAM
}

void UI_RedrawBoard(const uint8_t board[9], uint8_t youScore, uint8_t aiScore)
{
    OLED_Clear();
    ui_draw_side_info(youScore, aiScore);
    ui_draw_board();
    for (uint8_t i = 0; i < 9; i++)
    {
        if (board[i] == 1)
            UI_DrawPieceAt(i, 1);
        else if (board[i] == 2)
            UI_DrawPieceAt(i, 2);
    }
    OLED_Refresh();
}

void UI_DrawWinLine(uint8_t lineIndex)
{
    int16_t x0, y0, x1, y1;
    int16_t dx, dy, px, py, yShift;
    if (lineIndex > 7)
    {
        return;
    }

    ui_get_win_line_points(lineIndex, &x0, &y0, &x1, &y1, &dx, &dy, &px, &py, &yShift);
    ui_draw_parallel_line(x0, y0 + yShift, x1, y1 + yShift, 0, 0);
    ui_draw_parallel_line(x0, y0 + yShift, x1, y1 + yShift, px, py);
    ui_draw_parallel_line(x0, y0 + yShift, x1, y1 + yShift, (int16_t)(-px), (int16_t)(-py));
    ui_refresh_region(40, 86, 0, 3);
}

static void ui_draw_parallel_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t ox, int16_t oy)
{
    int16_t sx0 = ui_clamp_i16(x0 + ox, 0, 127);
    int16_t sy0 = ui_clamp_i16(y0 + oy, 0, 31);
    int16_t sx1 = ui_clamp_i16(x1 + ox, 0, 127);
    int16_t sy1 = ui_clamp_i16(y1 + oy, 0, 31);

    /* Ensure the line drawing function always receives points in a
       consistent order. Some OLED drivers assume x1 >= x0; for
       anti-diagonal lines we may have sx0 > sx1 so swap the points
       (keeping paired x/y) to avoid missing or inverted lines. */
    if (sx0 > sx1)
    {
        int16_t tx = sx0;
        sx0 = sx1;
        sx1 = tx;
        int16_t ty = sy0;
        sy0 = sy1;
        sy1 = ty;
    }

    /* Draw directly into OLED_GRAM to avoid inconsistencies in
       external OLED_DrawLine implementations. Use Bresenham. */
    ui_draw_line_gram(sx0, sy0, sx1, sy1);
}

static void ui_set_gram_point(uint8_t x, uint8_t y)
{
    if (x > 127 || y > 31) return;
    uint8_t page = (uint8_t)(y / 8);
    uint8_t bit = (uint8_t)(y % 8);
    OLED_GRAM[x][page] |= (uint8_t)(1U << bit);
}

static void ui_draw_line_gram(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    int16_t dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t dy = -((y1 > y0) ? (y1 - y0) : (y0 - y1));
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx + dy; /* error value e_xy */

    int16_t x = x0;
    int16_t y = y0;
    while (1)
    {
        ui_set_gram_point((uint8_t)x, (uint8_t)y);
        if (x == x1 && y == y1) break;
        int16_t e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            x += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y += sy;
        }
    }
}

static int16_t ui_clamp_i16(int16_t value, int16_t minValue, int16_t maxValue)
{
    if (value < minValue)
    {
        return minValue;
    }
    if (value > maxValue)
    {
        return maxValue;
    }
    return value;
}

static void ui_get_win_line_points(uint8_t lineIndex, int16_t *x0, int16_t *y0, int16_t *x1, int16_t *y1, int16_t *dx, int16_t *dy, int16_t *px, int16_t *py, int16_t *yShift)
{
    static const uint8_t lineCells[8][2] = {
        {0, 2}, {3, 5}, {6, 8}, {0, 6},
        {1, 7}, {2, 8}, {0, 8}, {2, 6}
    };

    uint8_t sx, sy, ex, ey;
    ui_cell_center(lineCells[lineIndex][0], &sx, &sy);
    ui_cell_center(lineCells[lineIndex][1], &ex, &ey);

    *dx = 0;
    *dy = 0;
    *px = 0;
    *py = 0;
    *yShift = 0;

    if (lineIndex < 3)
    {
        *dx = 1;
        *dy = 0;
        *px = 0;
        *py = 1;
    }
    else if (lineIndex < 6)
    {
        *dx = 0;
        *dy = 1;
        *px = (int16_t)-1;
        *py = 0;
        *yShift = 1;
    }
    else if (lineIndex == 6)
    {
        *dx = 1;
        *dy = 1;
        *px = (int16_t)-1;
        *py = 1;
        *yShift = 1;
    }
    else
    {
        *dx = 1;
        *dy = (int16_t)-1;
        *px = 1;
        *py = 1;
        *yShift = 1;
    }

     /* Extend endpoints by 4 pixels along the actual vector between
         the two cell centers. Using the sign of (ex - sx)/(ey - sy)
         ensures extension goes outward from each end regardless of
         which cell was listed first in lineCells. */
     int16_t sdx = (ex > sx) ? 1 : ((ex < sx) ? -1 : 0);
     int16_t sdy = (ey > sy) ? 1 : ((ey < sy) ? -1 : 0);

     *x0 = (int16_t)sx - (sdx * 4);
     *y0 = (int16_t)sy - (sdy * 4);
     *x1 = (int16_t)ex + (sdx * 4);
     *y1 = (int16_t)ey + (sdy * 4);
}

static void ui_draw_diamond(uint8_t x, uint8_t y, uint8_t radius)
{
    for (int8_t dx = (int8_t)(-radius); dx <= (int8_t)radius; dx++)
    {
        int8_t offset = (int8_t)(radius - ui_abs_i8(dx));

        if (offset == 0)
        {
            OLED_DrawPoint((uint8_t)(x + dx), y);
            continue;
        }

        for (int8_t dy = (int8_t)(-offset); dy <= offset; dy++)
        {
            if (dy == -offset || dy == offset)
            {
                OLED_DrawPoint((uint8_t)(x + dx), (uint8_t)(y + dy));
            }
        }
    }
}

static int8_t ui_abs_i8(int8_t value)
{
    return (value < 0) ? (int8_t)(-value) : value;
}
