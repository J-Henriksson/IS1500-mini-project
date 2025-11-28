/*
 * File:        draw_screen.c
 * Author:      Viktor Eriksson
 * Last Edited: 2025-11-21
 *
 * Description:
 *     Handles the main game loop and game state
 * 
 * Notes:
 *     each "tick" is activated by an interrupt from the hardware timer
 */

extern void _enable_interrupt();
extern void print(const char*);
extern void print_dec(unsigned int);
extern void draw_init(); // draw_screen.c
extern void clear_screen(); //draw_screen.c
extern void swap_buffers(); //draw_screen.c
extern void draw_grid (unsigned char red, 
    unsigned char green, unsigned char blue); // draw_screen.c
extern void draw_X(int x, int y, unsigned char red, 
    unsigned char green, unsigned char blue); // draw_screen.c
extern void draw_O(int x, int y, unsigned char red, 
    unsigned char green, unsigned char blue); // draw_screen.c
void draw_square(int column, int row, unsigned char red, 
    unsigned char green, unsigned char blue); // draw_screen.c

//timer control registers
volatile unsigned int* timer_status  = (volatile unsigned int*) 0x04000020;
volatile unsigned int* timer_control = (volatile unsigned int*) 0x04000024;
volatile unsigned int* timer_periodl = (volatile unsigned int*) 0x04000028;
volatile unsigned int* timer_periodh = (volatile unsigned int*) 0x0400002C;

//button register
volatile unsigned int* button_data = (volatile unsigned int*) 0x040000D0;
volatile unsigned int* button_edge = (volatile unsigned int*) 0x040000DC;

//switches register
volatile unsigned int* switches = (volatile unsigned int*) 0x04000010;


void timer_init() {
  // write 479 999 to the period register (16 ms - 1 extra cycle)
   *timer_periodl = 0x52FF; // lower 16 bits
   *timer_periodh = 0x0007; // upper 16 bits
  
   *timer_control = 0x0007; // START, CONT, ITO to 1, others to 0

   _enable_interrupt();
}

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

void switch_position(int* x, int* y)
{
    int x_bits = (*switches >> 7) & 0x7 ;
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

int main(int argc, char const *argv[])
{
    timer_init();
    draw_init();
    draw_grid(7,7,3);


    while(1) {}
}


void handle_interrupt()
{
    if (*timer_status & 0x1) 
    {
        *timer_status  = 0x0;   // clear TO
    }

    swap_buffers();

    clear_screen();
    draw_grid(7,7,3);
    
    int x = 0;
    int y = 0;
    switch_position(&x, &y);

    if (x > -1 && y > -1)
    {
        draw_square(x, y, 0, 7, 0);
    }
}