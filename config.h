/*
 * snake - simple console snake game 
 * Copyright (C) 2017-2020 Alessandro Righi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* regulate the speed of the game, lower = faster */
const int speed = 60;

/* show the border of the screen; if 0 the snake will wrap around */
const int limit = 1;

/* initial snake length */
const int initial_length = 10;

/* how many units to advance when SPACE is pressed */
const int space_advance_by = 5;

/* how many units to grow on $ powerup */
const int powerup_grow = 1;

/* how many units to grow on % powerup */
const int super_powerup_grow = 15;

/* how many units to decrese on * bomb */
const int bomb_decrease = 25;

/* change the various symbols for the game */
enum {
	SNAKE_HEAD = '@',
	SNAKE_BODY = '#',
	POWERUP = '$',
	SUPER_POWERUP = '%',
	BOMB = '*',
	SPACE = ' ', 
};

/* here you can change the color used in the game */
void
init_color_pairs()
{
	init_pair(COLOR_HEAD, COLOR_BLUE, -1);
	init_pair(COLOR_BODY, COLOR_CYAN, -1);
	init_pair(COLOR_SUPER_POWERUP, COLOR_YELLOW, -1);
	init_pair(COLOR_POWERUP, COLOR_GREEN, -1);
	init_pair(COLOR_BOMB, COLOR_RED, -1);
}

/* change the way the next level is computed */ 
int
level()
{
	return length / 30;	
}
