/*
 * File:        draw_screen.c
 * Author:      Viktor Eriksson
 * Last Edited: 2025-11-11
 *
 * Description:
 *     Handles all VGA draw operations for the tic-tac-toe game.
 * 
 * Notes:
 *     Uses 8-bit RGB (3-3-2) color encoding.
 */

volatile char *VGA = (volatile char *) 0x08000000; //address to VGA screen buffer

#define CELL_SIZE       51
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240
#define LINE_WIDTH      1

//runtime calculated constants (see draw_init())
int BOARD_OFFSET_X;
int BOARD_OFFSET_Y;

void draw_init()
{
     BOARD_OFFSET_X = (SCREEN_WIDTH - (CELL_SIZE*3 + 2*LINE_WIDTH))/2;
     BOARD_OFFSET_Y = (SCREEN_HEIGHT - (CELL_SIZE*3 + 2*LINE_WIDTH))/2;
}

/**
 * @brief Draw a single pixel on the VGA display.
 *
 * @param x      X coordinate (0–319)
 * @param y      Y coordinate (0–239)
 * @param red    Red component (0–7)
 * @param green  Green component (0–7)
 * @param blue   Blue component (0–3)
 *
 * This function writes an 8-bit RGB-332 encoded color value
 * directly to the VGA screen buffer.
 */ 
void draw_pixel (int x, int y, unsigned char red, 
    unsigned char green, unsigned char blue)
{
    unsigned char rgb = (red << 5) + (green << 2) + blue;
    VGA[x +320*y] = rgb;
}

/**
 * @brief Draw the game grid.
 *
 * @param red    Red component (0–7)
 * @param green  Green component (0–7)
 * @param blue   Blue component (0–3)
 *
 * This function draws the game grid in the given 8-bit RGB color.
 */ 
void draw_grid (unsigned char red, 
    unsigned char green, unsigned char blue)
{
    //vertical lines
    for (int y = BOARD_OFFSET_Y; y < (SCREEN_HEIGHT - BOARD_OFFSET_Y); y++)
    {
        for (int i = 0; i < 2; i++)
        {
            draw_pixel(BOARD_OFFSET_X + (i+1)*CELL_SIZE + i*LINE_WIDTH, y, red, green, blue);
        }
    }
    
    //horizontal lines
    for (int x = BOARD_OFFSET_X; x < (SCREEN_WIDTH - BOARD_OFFSET_X); x++)
    {
        for (int i = 0; i < 2; i++)
        {
            draw_pixel(x, BOARD_OFFSET_Y + (i+1)*CELL_SIZE + i*LINE_WIDTH, red, green, blue);
        }
    }
}
/**
 * @brief Draw a X-peice on the board
 *
 * @param row    row coordinate (0–2)
 * @param column column coordinate (0–2)
 * @param red    Red component (0–7)
 * @param green  Green component (0–7)
 * @param blue   Blue component (0–3)
 *
 * This function draws an X game piece in the given grid coordinates
 * and given 8-bit RGB color.
 */
void draw_X(int row, int column, unsigned char red, 
    unsigned char green, unsigned char blue)
{
    int cell_x = BOARD_OFFSET_X + column*(CELL_SIZE + LINE_WIDTH);
    int cell_y = BOARD_OFFSET_Y + row*(CELL_SIZE + LINE_WIDTH);

    for (int i = 5; i < (CELL_SIZE - 5); i++)
    {
        for (int j = -1; j < 2; j++)
        {
            draw_pixel((cell_x+j) + i, cell_y + i, red, green, blue);
            draw_pixel((cell_x+j) + i, cell_y + (CELL_SIZE - i - 1), red, green, blue);   
        }
    }
}

/**
 * @brief Draw a O-peice on the board
 *
 * @param row    row coordinate (0–2)
 * @param column column coordinate (0–2)
 * @param red    Red component (0–7)
 * @param green  Green component (0–7)
 * @param blue   Blue component (0–3)
 *
 * This function draws an O game piece in the given grid coordinates
 * and given 8-bit RGB color.
 */
void draw_O(int row, int column, unsigned char red, 
    unsigned char green, unsigned char blue)
{
    int cell_x = BOARD_OFFSET_X + column*(CELL_SIZE + LINE_WIDTH);
    int cell_y = BOARD_OFFSET_Y + row*(CELL_SIZE + LINE_WIDTH);
    
    int center_x = cell_x + CELL_SIZE/2;
    int center_y = cell_y + CELL_SIZE/2;
    int radius = CELL_SIZE/2 - 5;

    for (int x = -radius; x <= radius; x++)
    {
        for (int y = -radius; y <= radius; y++)
        {
            int distance2 = x*x + y*y;

            if (distance2 >= (radius-1)*(radius-1) && distance2 <= (radius+1)*(radius+1))
            {
                draw_pixel(center_x + x, center_y + y, red, green, blue);
            }
            
        }
        
    }
    
}

//main for testing purposes
/*
int main(int argc, char const *argv[])
{
    draw_init();

    draw_grid(7,7,3);

    draw_X(0, 0, 0, 0, 3);
    draw_O(1, 1, 7, 0, 0);


    while(1){};
}*/

void handle_interrupt()
{}
