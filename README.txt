READ ME — Tic-Tac-Toe for DE10-Lite

1. Overview ----------------------------------------------------------------------------

This program implements a two-player Tic-Tac-Toe game on the DE10-Lite FPGA board using C.
The game uses the slide switches to select a board position, the pushbutton to confirm moves, and the VGA output to display the board and pieces.
A custom VGA drawing library is used for rendering the grid, X and O markers, and other visual elements.
The 7-segment displays show the current score for each player.

2. Compilation -------------------------------------------------------------------------

To compile the program, place the project files together with the course-provided compilation files.
Then compile simply by running:

make

3. Running the Program ------------------------------------------------------------------

To upload and run the compiled program on the DTEK-V board, you must have the DTEK-V toolchain installed. 
Then run:

dtekv-run [path]/[to]/main.bin

The board must be connected to the computer via USB-Blaster and to a monitor via VGA (or VGA-to-HDMI adapter).

4. How to Play ---------------------------------------------------------------------------

Switches 0–2 select the row (0–2).

Switches 7–9 select the column (0–2).

The pushbutton (KEY1) confirms the move.

X initially plays first, then the player who lost starts the next round.

A green square indicates the currently selected cell.

Game outcomes:

When a player wins, the three winning pieces are highlighted by drawing all other pieces
in white.

When the game ends in a draw, all pieces are shown in white.

Press the button again to reset the game for a new round.

The total score for each player is displayed on the 7-segment display. 
player X's score is on the left, player O's on the right.

