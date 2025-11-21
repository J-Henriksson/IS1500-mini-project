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
extern void draw_init(); // draw_screen.c
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


void timer_init() {
  // write 479 999 to the period register (16 ms - 1 extra cycle)
   *timer_periodl = 0x52FF; // lower 16 bits
   *timer_periodh = 0x0007; // upper 16 bits
  
   *timer_control = 0x0007; // START, CONT, ITO to 1, others to 0

   _enable_interrupt();
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
        print("interrupt");
    }
}