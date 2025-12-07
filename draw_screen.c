/*
 * File:        draw_screen.c
 * Author:      Viktor Eriksson
 * Last Edited: 2025-12-04
 *
 * Description:
 *     Handles all VGA draw operations for the tic-tac-toe game.
 * 
 * Notes:
 *     Uses 8-bit RGB (3-3-2) color encoding.
 */
extern void print(const char*);
extern void print_dec(unsigned int);
void print_hex32 ( unsigned int);

//VGA 
volatile unsigned char* VGA = (volatile unsigned char*) 0x08000000;

//VGA screen_buffer base
#define VGA_BASE 0x08000000

//game constants
#define CELL_SIZE       51
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240
#define LINE_WIDTH      1
#define TURN_INDICATOR_SIZE 31
#define TURN_INDICATOR_OFFSET 20

//runtime calculated constants (see draw_init())
int BOARD_OFFSET_X;
int BOARD_OFFSET_Y;

//calculate run-time constants
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
void draw_pixel(int x, int y, unsigned char red, 
    unsigned char green, unsigned char blue)
{
    unsigned char rgb = (red << 5) + (green << 2) + blue;
    VGA[x +320*y] = rgb;
}

/**
 * @brief clears the screen.
 *
 * Clears the screen by filling the screen with black
 */ 
void clear_screen()
{
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
        VGA[i] = 0; 
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
void draw_grid(unsigned char red, 
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
 * @brief Draw an X on the board
 *
 * @param start_x   x_coordinate
 * @param start_y   row coordinate (0–2)
 * @param red       Red component (0–7)
 * @param green     Green component (0–7)
 * @param blue      Blue component (0–3)
 *
 * This function draws an X on the board with the given size and the given (x,y) position 
 * as the top left corner in the given 8-bit RGB color.
 */
void draw_X(int start_x, int start_y, int size, unsigned char red, 
    unsigned char green, unsigned char blue)
{
    for (int i = 5; i < (size - 5); i++)
    {
        for (int j = -1; j < 2; j++)
        {
            draw_pixel((start_x+j) + i, start_y + i, red, green, blue);
            draw_pixel((start_x+j) + i, start_y + (size - i - 1), red, green, blue);   
        }
    }
}

/**
 * @brief Draw an O on the board
 *
 * @param start_x   x_coordinate
 * @param start_y   row coordinate (0–2)
 * @param red       Red component (0–7)
 * @param green     Green component (0–7)
 * @param blue      Blue component (0–3)
 *
 * This function draws an O on the board with the given size and the given (x,y) position 
 * as the top left corner in the given 8-bit RGB color.
 */
void draw_O(int start_x, int start_y, int size, unsigned char red, 
    unsigned char green, unsigned char blue)
{
    int center_x = start_x + size/2;
    int center_y = start_y + size/2;
    int radius = size/2 - 5;

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

/**
 * @brief Draw a square on the board
 *
 * @param start_x x_coordinate
 * @param start_y   row coordinate (0–2)
 * @param red    Red component (0–7)
 * @param green  Green component (0–7)
 * @param blue   Blue component (0–3)
 *
 * This function draws a square on the board with the given size and the given (x,y) position 
 * as the top left corner in the given 8-bit RGB color.
 */
 void draw_square(int start_x, int start_y,int size, unsigned char red, 
    unsigned char green, unsigned char blue)
{
    for (int y = 0; y < size; y++)
    {
        draw_pixel(start_x, (start_y + y), red, green, blue);
        draw_pixel((start_x + size - 1), (start_y +y), red, green, blue);
    }

    for (int x = 0; x < size; x++)
    {
        draw_pixel((start_x + x), start_y, red, green, blue);
        draw_pixel((start_x + x), (start_y + size -1), red, green, blue);
    }
}

void draw_cursor(int column, int row)
{
    int cell_x = BOARD_OFFSET_X + column*(CELL_SIZE + LINE_WIDTH);
    int cell_y = BOARD_OFFSET_Y + row*(CELL_SIZE + LINE_WIDTH);

    draw_square(cell_x, cell_y, CELL_SIZE, 0, 7, 0);
}

/**
 * @brief Draws a marker indicating the turn
 *
 * @param player the turn to display (1 or 2)
 *
 * This function draws a small square containing the symbol of the current player to indicate
 * who's turn it is.
 */
void draw_turn_indicator(int player)
{
    draw_square(TURN_INDICATOR_OFFSET, TURN_INDICATOR_OFFSET, TURN_INDICATOR_SIZE, 7, 7, 3);
    if (player == 1)
    {
        draw_X(TURN_INDICATOR_OFFSET, TURN_INDICATOR_OFFSET, TURN_INDICATOR_SIZE, 0, 0, 3);
    }
    else
    {
         draw_O(TURN_INDICATOR_OFFSET, TURN_INDICATOR_OFFSET, TURN_INDICATOR_SIZE, 7, 0, 0);
    }
}



/**
 * @brief Draw a the game pieces on the board
 *
 * @param board 3x3 array of the board
 * @param win_cells 3x3 array containing the cells involved in a win
 * @param winner 0 = no winner, 1 or 2 is the winning player and 3 is a draw
 *
 * This function draws game pieces on the board based on a 3x3 array representing the board
 * if the game is over, some pieces will be drawn white to show the winner
 * if it's a draw all pieces will be white
 */
void draw_pieces(int board[3][3], int win_cells[3][3], int winner)
{
    for (int col = 0; col < 3; col++)
    {
        for (int row = 0; row < 3; row++)
        {
            int cell_x = BOARD_OFFSET_X + col*(CELL_SIZE + LINE_WIDTH);
            int cell_y = BOARD_OFFSET_Y + row*(CELL_SIZE + LINE_WIDTH);

            int cell = board[col][row];

            if (cell == 1)  // PLAYER_X
            {
                if ((winner && win_cells[col][row] != 1))
                    draw_X(cell_x, cell_y, CELL_SIZE, 7, 7, 3); 
                else 
                draw_X(cell_x, cell_y, CELL_SIZE, 0, 0, 3);  
            }
            else if (cell == 2)  // PLAYER_O
            {
                if (winner && win_cells[col][row] != 2)
                    draw_O(cell_x, cell_y, CELL_SIZE, 7, 7, 3);  
                else
                    draw_O(cell_x, cell_y, CELL_SIZE, 7, 0, 0);  
            }
        }
    }
}