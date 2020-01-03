# snake
Simple cli ASCII snake game, written in a boring afternoon with C and ncurses.

## HOW TO PLAY

- move the snake with arrow keys or vim keys (h,j,k,l)
- SPACE advances snake 5 position in the current direction
- press p to pause the game
- collect $ to grow the snake of 1 unit
- collect % to grow the snake of 15 units
- avoid \*, since they decrese the snake length by 25 units
- hitting the borders of the screen make you loose the game

## Personalization 

The game parameters can be configured by editing the `config.h` file, as is done for
suckless.org utilities. Have fun tweaking it! 

## Installation
Compile with `make`. Ncurses is required, if you are on Debian/Ubuntu install the package `libncurses-dev`. 
On other Linux distributions and UNIX-like systems it should be installed by default. 

