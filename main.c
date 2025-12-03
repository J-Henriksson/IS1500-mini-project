/*
 * File:        main.c
 * Author:      Joel Henriksson
 * Last Edited: 2025-12-03
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
extern void draw_pieces(int board[3][3], int draw); //draw_screen.c
extern void draw_turn_indicator(int player); // draw_screen.c

// button register
volatile unsigned int* button = (volatile unsigned int*) 0x040000D0;

// switches register
volatile unsigned int* switches = (volatile unsigned int*) 0x04000010;

// Game state

#define EMPTY    0
#define PLAYER_X 1
#define PLAYER_O 2

// board[row][col], where row = y (0–2), col = x (0–2)
int board[3][3];
int win_cells[3][3];
int current_player = PLAYER_X;
int game_over = 0;  // 0 = playing, 1 = finished
int winner = 0;     // 0 = none, 1 = X, 2 = O, 3 = draw


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
void switch_position(int* x, int* y)
{
    int x_bits = (*switches >> 7) & 0x7;
    int y_bits = *switches & 0x7;

    // Decode X
    if      (x_bits == 0x4) *x = 0;
    else if (x_bits == 0x2) *x = 1;
    else if (x_bits == 0x1) *x = 2;
    else                    *x = -1; // invalid (none or multiple)

    // Decode Y
    if      (y_bits == 0x4) *y = 0;
    else if (y_bits == 0x2) *y = 1;
    else if (y_bits == 0x1) *y = 2;
    else                    *y = -1; // invalid
}

// Initialize / reset the game state

void game_init(void)
{
    clear_screen();
    draw_grid(7,7,3);
    draw_turn_indicator(1);

    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            board[r][c] = EMPTY;
        }
    }

    current_player = PLAYER_X;
    game_over = 0;
    winner = 0;
}

// Check if someone has won or if it's a draw.
// Return: 0 = no winner yet, 1 = X wins, 2 = O wins, 3 = draw
int check_winner(void)
{   
    //clear win_cells
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            win_cells[r][c] = EMPTY;
        }
    }

    // rows
    for (int r = 0; r < 3; r++)
    {
        if (board[r][0] != EMPTY &&
            board[r][0] == board[r][1] &&
            board[r][1] == board[r][2])
        {
            int won = board[r][0];
            win_cells[r][0] = won;
            win_cells[r][1] = won;
            win_cells[r][2] = won;
            return won;
        }
    }

    // columns
    for (int c = 0; c < 3; c++)
    {
        if (board[0][c] != EMPTY &&
            board[0][c] == board[1][c] &&
            board[1][c] == board[2][c])
        {
            int won = board[0][c];
            win_cells[0][c] = won;
            win_cells[1][c] = won;
            win_cells[2][c] = won;
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
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            if (board[r][c] == EMPTY)
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

// Main + interrupt logic

int main(int argc, char const *argv[])
{
    // Initialize drawing and game state *before* enabling interrupts
    draw_init();
    game_init();

    int y = 0;
    int x = 0;
    while(1) 
    {
        int state_updated = 0;

        int new_x = 0;
        int new_y = 0;
        switch_position(&new_x, &new_y);
        if (new_x != x || new_y != y) //check if switch position changed
        {
            state_updated = 1;
            x = new_x;
            y = new_y;
        }
        
        int pressed = button_press();

        if (!game_over)
        {
            // Only attempt mark placement if cursor is valid
            if (x > -1 && y > -1)
            {
                if (pressed && board[y][x] == EMPTY)
                {
                    board[y][x] = current_player;  

                    int result = check_winner();
                    if (result != 0)
                    {
                        // Game finished
                        game_over = 1;
                        winner = result;
                    }
                    else
                    {
                        // Switch player
                        if (current_player == PLAYER_X)
                            current_player = PLAYER_O;
                        else
                            current_player = PLAYER_X;
                    }
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
            clear_screen();
            
            if (!game_over)
            {
                draw_grid(7, 7, 3);
                draw_pieces(board, 0);
                draw_turn_indicator(current_player);
                if (x > -1 && y > -1)
                {
                draw_cursor(x, y);
                }
            }
            else if (winner == 3)
            {
                draw_grid(7,7,3);
                draw_pieces(board, 1);

            }
            else
            {
                draw_pieces(win_cells, 0);
            }
        }
    }
}

// This is called from boot.s on external interrupt
void handle_interrupt()
{}
