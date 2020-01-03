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

#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ncurses.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>

enum color_pairs {
	COLOR_HEAD = 1,
	COLOR_BODY,
	COLOR_POWERUP,
	COLOR_SUPER_POWERUP,
	COLOR_BOMB,
};

enum {
	UP,
	DOWN,
	LEFT,
	RIGHT,
} direction;

enum {
	MAX_SIZE_X = 512,
	MAX_SIZE_Y = 256,
	MAX_SNAKE_LENGTH = 4096,
};

struct point {
	int x, y;
} position[MAX_SNAKE_LENGTH], size;

int screen[MAX_SIZE_Y][MAX_SIZE_X], started, length, head_pos, tail_pos;

#include "config.h"

void
print_center(int i, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	move(size.y / 2 + i, size.x / 2 - (int) strlen(fmt) / 2);
	vwprintw(stdscr, fmt, args);
	va_end(args);
}

void
add_powerup(void)
{
	int x, y;

	do {
		x = rand() % size.x;
		y = rand() % size.y;
	} while (screen[y][x] != SPACE);

	switch (rand() % 10) {
	case 7:
		screen[y][x] = SUPER_POWERUP;
		break;
	case 3:
		screen[y][x] = BOMB;
		add_powerup();
		break;
	default:
		screen[y][x] = POWERUP;
	}
}

void
remove_tail(void)
{
	screen[position[tail_pos].y][position[tail_pos].x] = SPACE;
	tail_pos = (tail_pos + 1) % MAX_SNAKE_LENGTH;
}

void
add_head(struct point head)
{
	screen[position[head_pos].y][position[head_pos].x] = SNAKE_BODY;
	screen[head.y][head.x] = SNAKE_HEAD;
	head_pos = (head_pos + 1) % MAX_SNAKE_LENGTH;
	position[head_pos].y = head.y;
	position[head_pos].x = head.x;
}

void
refresh_screen(void)
{
	int y, x, color;

	for (y = 0; y < size.y; y++) {
		for (x = 0; x < size.x; x++) {
			switch (screen[y][x]) {
			case SNAKE_HEAD: color = COLOR_HEAD; break;
			case SNAKE_BODY: color = COLOR_BODY; break;
			case POWERUP: color = COLOR_POWERUP; break;
			case SUPER_POWERUP: color = COLOR_SUPER_POWERUP; break;
			case BOMB: color = COLOR_BOMB; break;
			default: color = 0;
			}

			attron(COLOR_PAIR(color));
			mvaddch(y, x, screen[y][x]);
			attroff(COLOR_PAIR(color));
		}
	}

	if (limit)
		mvprintw(0, 4, " SCORE %d; LEVEL %d ", length, level());
	refresh();
}

void
dup_tail(void)
{
	struct point tail = position[tail_pos];

	tail_pos = (tail_pos - 1) % MAX_SNAKE_LENGTH;
	position[tail_pos] = tail;
	length++;
}

void
init_game(void)
{
	int y, x;

	size.y = LINES;
	size.x = COLS;
	length = 0;
	started = 0;
	head_pos = 0;
	tail_pos = 0;
	direction = RIGHT;

	for (y = 0; y < size.y; y++) {
		for (x = 0; x < size.x; x++) {
			screen[y][x] = SPACE;
		}
	}

	if (limit) {
		for (x = 0; x < size.x; x++) {
			screen[0][x] = ACS_HLINE;
			screen[size.y - 1][x] = ACS_HLINE;
		}

		for (y = 0; y < size.y; y++) {
			screen[y][0] = ACS_VLINE;
			screen[y][size.x - 1] = ACS_VLINE;
		}

		screen[0][0] = ACS_ULCORNER;
		screen[0][size.x - 1] = ACS_URCORNER;
		screen[size.y - 1][0] = ACS_LLCORNER;
		screen[size.y - 1][size.x - 1] = ACS_LRCORNER;
	}

	for (head_pos = 0; head_pos < initial_length; head_pos++) {
		position[head_pos].x = size.x / 2;
		position[head_pos].y = size.y / 2;
		screen[position[head_pos].y][position[head_pos].x] = SNAKE_BODY;
		length++;
	}

	head_pos--;
	screen[position[head_pos].y][position[head_pos].x] = SNAKE_HEAD;
	add_powerup();
	add_powerup();
	refresh_screen();
}

void
game_lost(void)
{
	int ch;

	print_center(-1, "You lose!");
	print_center(0, "Your score: %d", length);
	print_center(1, "Play new game ? (y/n)");

	refresh();

	while ((ch = getch()) != 'y') {
		if (ch == 'n')
			exit(EXIT_SUCCESS);
	}

	init_game();
}

void
advance(void)
{
	int i;
	struct point head = position[head_pos];

	started = 1;

	if (direction == LEFT || direction == RIGHT)
		timeout(speed - (unsigned) level() * 3);
	else
		timeout(speed * 2 - (unsigned) level() * 6);

	switch (direction) {
	case UP:
		head.y -= 1;
		if (head.y < 0)
			head.y = size.y - 1;
		break;
	case DOWN:
		head.y += 1;
		if (head.y == size.y)
			head.y = 0;
		break;
	case LEFT:
		head.x -= 1;
		if (head.x < 0)
			head.x = size.x - 1;
		break;
	case RIGHT:
		head.x += 1;
		if (head.x == size.x)
			head.x = 0;
		break;
	}

	switch (screen[head.y][head.x]) {
	case SPACE:
		add_head(head);
		remove_tail();
		break;
	case SUPER_POWERUP:
		for (i = 0; i < super_powerup_grow - powerup_grow; i++)
			dup_tail();
		/* fall-thru */
	case POWERUP:
		for (i = 1; i < powerup_grow; i++)
			dup_tail();
		add_head(head);
		add_powerup();
		length++;
		break;
	case BOMB:
		add_head(head);
		remove_tail();
		for (i = 0; i < bomb_decrease; i++) {
			remove_tail();
			if (--length == 0)
				break;
		}
		if (length)
			break;
		/* fall-thru */
	default:
		add_head(head);
		remove_tail();
		for (i = 0; i < 5; i++) {
			screen[head.y][head.x] = SUPER_POWERUP;
			refresh_screen();
			usleep(140000);
			screen[head.y][head.x] = SNAKE_HEAD;
			refresh_screen();
			usleep(140000);
		}
		game_lost();
	}
	refresh_screen();
}

void
confirm_exit(void)
{
	int ch;

	print_center(0, "Are you sure you want to quit ? (y/n)");
	refresh();
	timeout(0);
	while ((ch = getch()) != 'n') {
		if (ch == 'y')
			exit(EXIT_SUCCESS);
	}
	advance();
}

void
init_curses(void)
{
	initscr();
	cbreak();
	keypad(stdscr, 1);
	noecho();
	curs_set(0);

	use_default_colors();
	start_color();
	init_color_pairs();

	atexit((void(*)(void)) endwin);
}

int
main(void)
{
	int i;

	srand((unsigned) time(NULL));
	init_curses();
	init_game();

	for (;;) {
		switch (getch()) {
		case 'k':
		case KEY_UP:
			if (direction != DOWN || !started)
				direction = UP;
			break;
		case 'j':
		case KEY_DOWN:
			if (direction != UP || !started)
				direction = DOWN;
			break;
		case 'h':
		case KEY_LEFT:
			if (direction != RIGHT || !started)
				direction = LEFT;
			break;
		case 'l':
		case KEY_RIGHT:
			if (direction != LEFT || !started)
				direction = RIGHT;
			break;
		case SPACE:
			for (i = 0; i < 5; i++)
				advance();
			break;
		case 'p':
			print_center(0, "Game paused - press 'p' to resume");
			while (getch() != 'p')
				continue;
			break;
		case 'q':
			confirm_exit();
		}
		advance();
	}
}
