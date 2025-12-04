/*
 * File:        main.c
 * Author:      Joel Henriksson
 * Last Edited: 2025-12-04
 *
 * Description:
 *     Handles the main game loop and game state for tic-tac-toe.
 * 
 * Notes:
 *     Each "tick" is activated by an interrupt from the hardware timer.
 */

extern void print(const char*);
extern void print_dec(unsigned int);
extern void draw_init();    // draw_screen.c
extern void clear_screen(); // draw_screen.c
extern void draw_grid (unsigned char red, 
    unsigned char green, unsigned char blue); // draw_screen.c
extern void draw_cursor(int column, int row); // draw_screen.c
extern void draw_square(int column, int row, unsigned char red, 
    unsigned char green, unsigned char blue); // draw_screen.c
extern void draw_pieces(int board[3][3], int win_cells[3][3], int winner); //draw_screen.c
extern void draw_turn_indicator(int player); // draw_screen.c

// button register
volatile unsigned int* button = (volatile unsigned int*) 0x040000D0;

// switches register
volatile unsigned int* switches = (volatile unsigned int*) 0x04000010;

//seven segment display numbers and base adress
static const int seven_seg_digits[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
volatile unsigned char* seven_seg_base = (volatile unsigned char*) 0x04000050;

// Game state
#define EMPTY    0
#define PLAYER_X 1
#define PLAYER_O 2

// board[row][col], where row = x (0–2), col = y (0–2)
int board[3][3];
int win_cells[3][3];
int current_player;;
int winner;     // 0 = none, 1 = X, 2 = O, 3 = draw
int X_score = 0;
int O_score = 0;


// Rising-edge detector on button 0
int button_press()
{
    static int button_pressed = 0;
    int current = *button & 0x1;
    if ((current) && !button_pressed)
    {
        button_pressed = 1; 
        return 1;
    }
    if (!current)
    {
        button_pressed = 0;
    }
    return 0;
}

// Decode switches into (x,y) cell coordinates
void switch_position(int* col, int* row)
{
    int col_bits = (*switches >> 7) & 0x7;
    int row_bits = *switches & 0x7;

    // Decode X
    if      (col_bits == 0x4) *col = 0;
    else if (col_bits == 0x2) *col = 1;
    else if (col_bits == 0x1) *col = 2;
    else                      *col = -1; // invalid (none or multiple)

    // Decode Y
    if      (row_bits == 0x4) *row = 0;
    else if (row_bits == 0x2) *row = 1;
    else if (row_bits == 0x1) *row = 2;
    else                      *row = -1; // invalid
}

// Initialize / reset the game state

void game_init(void)
{
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            board[r][c] = EMPTY;
        }
    }

    current_player = PLAYER_X;
    winner = 0;
}

// Check if someone has won or if it's a draw.
// Return: 0 = no winner yet, 1 = X wins, 2 = O wins, 3 = draw
int check_winner(void)
{   
    //clear win_cells
    for (int c = 0; c < 3; c++)
    {
        for (int r = 0; r < 3; r++)
        {
            win_cells[r][c] = EMPTY;
        }
    }

    // columns
    for (int c = 0; c < 3; c++)
    {
        if (board[c][0] != EMPTY &&
            board[c][0] == board[c][1] &&
            board[c][1] == board[c][2])
        {
            int won = board[c][0];
            win_cells[c][0] = won;
            win_cells[c][1] = won;
            win_cells[c][2] = won;
            return won;
        }
    }

    // rows
    for (int r = 0; r < 3; r++)
    {
        if (board[0][r] != EMPTY &&
            board[0][r] == board[1][r] &&
            board[1][r] == board[2][r])
        {
            int won = board[0][r];
            win_cells[0][r] = won;
            win_cells[1][r] = won;
            win_cells[2][r] = won;
            return won;
        }
    }

    // diagonals
    if (board[0][0] != EMPTY &&
        board[0][0] == board[1][1] &&
        board[1][1] == board[2][2])
    {
        int won = board[0][0];
         win_cells[0][0] = won;
        win_cells[1][1] = won;
        win_cells[2][2] = won;
        return won;
    }

    if (board[0][2] != EMPTY &&
        board[0][2] == board[1][1] &&
        board[1][1] == board[2][0])
    {
        int won = board[0][2];
            win_cells[0][2] = won;
            win_cells[1][1] = won;
            win_cells[2][0] = won;
            return won;
    }

    // Check for draw: if no EMPTY cells remain
    int full = 1;
    for (int c = 0; c < 3; c++)
    {
        for (int r = 0; r < 3; r++)
        {
            if (board[c][r] == EMPTY)
            {
                full = 0;
                break;
            }
        }
        if (!full) break;
    }

    if (full) return 3;  // draw
    return 0;            // game continues
}

//updates the screen based on current game state
void update_screen(int col, int row)
{
    clear_screen();
    draw_grid(7, 7, 3);
    draw_pieces(board, win_cells, winner);
            
    if (!winner)
    { 
        draw_turn_indicator(current_player);
        if (col > -1 && row > -1)
            draw_cursor(col, row);
    }
    
}

//set a display to the given number (-1 turn the display off)
void set_displays(int display_number, int value) 
{
    if (display_number > -1 && display_number < 6) 
    {
        volatile char* address = (volatile char*) (seven_seg_base + (display_number * 0x10));
        if (value == -1)
            *address = 0xFF;
        else
            *address = seven_seg_digits[value];
    }
  
}

// Main game loop
int main(int argc, char const *argv[])
{
    // Initialize drawing and game state
    draw_init();
    game_init();

    //turn middle displays off
    set_displays(2, -1);
    set_displays(3, -1);

    int col = 0;
    int row = 0;
    while(1) 
    {
        int state_updated = 0;

        int new_col = 0;
        int new_row = 0;
        switch_position(&new_col, &new_row);
        if (new_col != col || new_row != row) //check if switch position changed
        {
            state_updated = 1;
            col = new_col;
            row = new_row;
        }
        
        int pressed = button_press();

        if (!winner)
        {
            // Only attempt mark placement if cursor is valid
            if (col > -1 && row > -1)
            {
                if (pressed && board[col][row] == EMPTY)
                {
                    board[col][row] = current_player;  

                    winner = 0;
                    winner = check_winner();
                    if (winner == 0)
                    {
                        // Switch player
                        if (current_player == PLAYER_X)
                            current_player = PLAYER_O;
                        else
                            current_player = PLAYER_X;
                    }
                    else if (winner == 1)
                        X_score++;
                    else if (winner == 2)
                        O_score++;
                       
                
                    

                    state_updated = 1;
                }
            }
        }
        else
        {
            // Game is over: any button press starts a new game
            if (pressed)
            {
                game_init();
                state_updated = 1;
            }
        }

        //redraw the screen when game state has been updated
        if (state_updated) 
        {
            update_screen(col, row);

            set_displays(0, O_score%10);
            set_displays(1, O_score/10);

            set_displays(4, X_score%10);
            set_displays(5, X_score/10);
            
        }
        
    }
}

// empty handle_interrupt
void handle_interrupt()
{}
