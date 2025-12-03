/*
 * File:        main.c
 * Author:      Viktor Eriksson (+ tic-tac-toe logic)
 * Last Edited: 2025-11-21
 *
 * Description:
 *     Handles the main game loop and game state for tic-tac-toe.
 * 
 * Notes:
 *     Each "tick" is activated by an interrupt from the hardware timer.
 */

extern void _enable_interrupt();
extern void print(const char*);
extern void print_dec(unsigned int);

extern void draw_init();    // draw_screen.c
extern void clear_screen(); // draw_screen.c
extern void swap_buffers(); // draw_screen.c
extern void draw_grid (unsigned char red, 
    unsigned char green, unsigned char blue); // draw_screen.c
extern void draw_square(int column, int row, unsigned char red, 
    unsigned char green, unsigned char blue); // draw_screen.c

// timer control registers
volatile unsigned int* timer_status  = (volatile unsigned int*) 0x04000020;
volatile unsigned int* timer_control = (volatile unsigned int*) 0x04000024;
volatile unsigned int* timer_periodl = (volatile unsigned int*) 0x04000028;
volatile unsigned int* timer_periodh = (volatile unsigned int*) 0x0400002C;

// button register
volatile unsigned int* button_data = (volatile unsigned int*) 0x040000D0;
volatile unsigned int* button_edge = (volatile unsigned int*) 0x040000DC;

// switches register
volatile unsigned int* switches = (volatile unsigned int*) 0x04000010;

// Game state

#define EMPTY    0
#define PLAYER_X 1
#define PLAYER_O 2

// board[row][col], where row = y (0–2), col = x (0–2)
int board[3][3];
int current_player = PLAYER_X;
int game_over = 0;  // 0 = playing, 1 = finished
int winner = 0;     // 0 = none, 1 = X, 2 = O, 3 = draw

// Helper functions

void timer_init() {
  // write 479 999 to the period register (16 ms - 1 extra cycle)
   *timer_periodl = 0x52FF; // lower 16 bits
   *timer_periodh = 0x0007; // upper 16 bits
  
   *timer_control = 0x0007; // START, CONT, ITO to 1, others to 0

   _enable_interrupt();
}

// Rising-edge detector on button 0
int button_press()
{
    static int button_pressed = 0;
    int current = *button_data & 0x1;
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
    if      (x_bits == 0x1) *x = 0;
    else if (x_bits == 0x2) *x = 1;
    else if (x_bits == 0x4) *x = 2;
    else                    *x = -1; // invalid (none or multiple)

    // Decode Y
    if      (y_bits == 0x1) *y = 0;
    else if (y_bits == 0x2) *y = 1;
    else if (y_bits == 0x4) *y = 2;
    else                    *y = -1; // invalid
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
    game_over = 0;
    winner = 0;
}

// Check if someone has won or if it's a draw.
// Return: 0 = no winner yet, 1 = X wins, 2 = O wins, 3 = draw
int check_winner(void)
{
    // rows
    for (int r = 0; r < 3; r++)
    {
        if (board[r][0] != EMPTY &&
            board[r][0] == board[r][1] &&
            board[r][1] == board[r][2])
        {
            return board[r][0];
        }
    }

    // columns
    for (int c = 0; c < 3; c++)
    {
        if (board[0][c] != EMPTY &&
            board[0][c] == board[1][c] &&
            board[1][c] == board[2][c])
        {
            return board[0][c];
        }
    }

    // diagonals
    if (board[0][0] != EMPTY &&
        board[0][0] == board[1][1] &&
        board[1][1] == board[2][2])
    {
        return board[0][0];
    }

    if (board[0][2] != EMPTY &&
        board[0][2] == board[1][1] &&
        board[1][1] == board[2][0])
    {
        return board[0][2];
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
    draw_grid(7, 7, 3);    // initial grid

    // Enable timer + interrupts last
    timer_init();

    while(1) {}
}

// This is called from boot.s on external interrupt
void handle_interrupt()
{
    // Timer interrupt?
    if (*timer_status & 0x1) 
    {
        *timer_status  = 0x0;   // clear TO
    }

    // Swap front/back buffers
    swap_buffers();

    clear_screen();
    draw_grid(7, 7, 3);


    int x = 0;
    int y = 0;
    switch_position(&x, &y);

    int pressed = button_press();

    if (!game_over)
    {
        // Only draw cursor square if the switch selection is valid
        if (x > -1 && y > -1)
        {
            // green selection square
            draw_square(x, y, 0, 7, 0);

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
            }
        }
    }
    else
    {
        // Game is over: any button press starts a new game
        if (pressed)
        {
            game_init();
        }
    }
}
