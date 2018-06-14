/* 
 * Snake game
 * Copyright (c) 2017-2018 Alessandro Righi - MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ncurses.h> 
#include <getopt.h>
#include <time.h>

enum {
	SNAKE_HEAD = '@',
	SNAKE_BODY = '#',
	POWERUP = '$',
	SUPER_POWERUP = '%',
	BOMB = '*',
	SPACE = ' ', 
};

enum direction {
	UP,
	DOWN,
	LEFT,
	RIGHT,
};

enum difficulty {
	EASY = 90,
	MEDIUM = 60,
	HARD = 30,
};

enum {
	MAX_SIZE_X = 512,
	MAX_SIZE_Y = 256,
	MAX_SNAKE_LENGTH = 4096,
};

struct point { 
	int x, y; 
};

static const char VERSIONSTRING[] = "Snake v1.2.1 (c) 2017 Alessandro Righi";
static const char USAGE[] = 
		"Usage: snake [-hvd]\n"
		"    -d (e)asy/(m)edium/(h)ard select difficulty\n"
		"    -h                        show this help\n"
		"    -v                        version string\n"
		"    -n                        no screen border\n"
		" * move the snake with arrow keys\n"
		" * space bar for extra speed (use with caution)\n"
		" * q for quitting the game\n"
		" * p pause the game\n"
		" * if you hit the borders of the screen, or your tail, you lose\n"		   
		" * powerups:\n"
		"     $ - increase snake length by 1\n"
		"     %% - increase snake length by 15\n"
		"     * - decrease snake length by 25"; 

static char score_filename[1024];
static chtype screen[MAX_SIZE_Y][MAX_SIZE_X];

static enum direction direction;
static enum difficulty difficulty = MEDIUM;

static struct point position[MAX_SNAKE_LENGTH];
static struct point size;

static bool limit = true;

static int length;
static int level;
static int score;
static int high_score;
static int head_pos;
static int tail_pos;

static void print_center(int i, const char *fmt, ...) 
{
	va_list args; 
	va_start(args, fmt);
	move(size.y / 2 + i, size.x / 2 - (int) strlen(fmt) / 2);
	vwprintw(stdscr, fmt, args);
	va_end(args);
}

static void load_score(void) 
{
	FILE *score_fp;

	snprintf(score_filename, sizeof score_filename, "%s/.snake_score", getenv("HOME"));
	
	if ((score_fp = fopen(score_filename, "r"))) {
		fscanf(score_fp, "%ud", &high_score);
		fclose(score_fp);
	} else {
		perror("Error loading score");
	}
}

static void save_score(void) 
{
	FILE *score_fp;

	if ((score_fp = fopen(score_filename, "w+"))) {
		fprintf(score_fp, "%ud\n", high_score);
		fclose(score_fp);
	} else {
		perror("Error saving score");
	}
}

static void add_powerup(void) 
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

static void remove_tail(void) 
{
	screen[position[tail_pos].y][position[tail_pos].x] = SPACE;
	tail_pos = (tail_pos + 1) % MAX_SNAKE_LENGTH;
}

static void add_head(struct point head) 
{
	screen[position[head_pos].y][position[head_pos].x] = SNAKE_BODY;
	screen[head.y][head.x] = SNAKE_HEAD;	
	head_pos = (head_pos + 1) % MAX_SNAKE_LENGTH;
	position[head_pos].y = head.y;
	position[head_pos].x = head.x;
}

static void print_screen(void) 
{	
	clear();

	for (int i = 0; i < size.y; i++) {
		for (int j = 0; j < size.x; j++) {
			addch(screen[i][j]);
		}
	}

	if (limit)
		mvprintw(0, 4, " SCORE %d; HIGH SCORE %u ", score, high_score);
	
	refresh();
}

static void dup_tail(void) 
{
	struct point tail = position[tail_pos];

	tail_pos = (tail_pos - 1) % MAX_SNAKE_LENGTH;
	position[tail_pos] = tail;
	length++;
	score++;
}

static void init_game(void) 
{	
	size.y = LINES;
	size.x = COLS;
	length = 0;
	head_pos = 0;
	tail_pos = 0;
	score = 0;
	direction = RIGHT;
	
	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			screen[y][x] = SPACE;
		}
	}

	if (limit) {
		for (int i = 0; i < size.x; i++) {
			screen[0][i] = ACS_HLINE;
			screen[size.y - 1][i] = ACS_HLINE;		
		}

		for (int i = 0; i < size.y; i++) {
			screen[i][0] = ACS_VLINE;
			screen[i][size.x - 1] = ACS_VLINE;		
		}

		screen[0][0] = ACS_ULCORNER;
		screen[0][size.x - 1] = ACS_URCORNER;
		screen[size.y - 1][0] = ACS_LLCORNER;
		screen[size.y - 1][size.x - 1] = ACS_LRCORNER;
	}

	for (head_pos = 0; head_pos < 7; head_pos++) {
		position[head_pos].x = size.x / 2;
		position[head_pos].y = size.y / 2;
		screen[position[head_pos].y][position[head_pos].x] = SNAKE_BODY;				
		length++;
	}

	head_pos--;
	screen[position[head_pos].y][position[head_pos].x] = SNAKE_HEAD;		
	add_powerup();
	add_powerup();
	print_screen();	
}

static void quit_game(void) 
{
	endwin();
	if (score > high_score) {
		printf("Congratulations! You beated the high score %d with %d\n", high_score, score);
		high_score = score;
	} 
	save_score();
}

static void game_lost(void) 
{	
	int ch;

	print_center(-1, "You lose!");
	
	if (score > high_score) {
		high_score = score;
		print_center(0, "Congratulations! New High Score %d!", score);
	} else {
		print_center(0, "Your score: %d", score);
	}

	print_center(1, "Play new game ? (y/n)");
	refresh();
 
	while ((ch = getch()) != 'y') {
		if (ch == 'n')
			exit(EXIT_SUCCESS);
	}

	init_game();
}

static void advance(void)
{
	struct point head = position[head_pos];

	if (!score)
		score = 1;
	
	if (direction == LEFT || direction == RIGHT)
		timeout(difficulty - (unsigned) level * 3);	
	else
		timeout(difficulty * 2 - (unsigned) level * 6);	

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
		for (int i = 0; i < 15; i++)
			dup_tail();
		/* fall-thru */
	case POWERUP:
		add_head(head);
		add_powerup();
		length++;
		score++;		
		break;
	case BOMB:
		add_head(head);
		remove_tail();
		score -= 35;
		for (int i = 0; i < 25; i++) {
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
		for (int i = 0; i < 5; i++) {
			screen[head.y][head.x] = SUPER_POWERUP;		
			print_screen();			
			usleep(140000);	
			screen[head.y][head.x] = SNAKE_HEAD;
			print_screen();
			usleep(140000);		
		}
		game_lost();
	}
	print_screen();
	level = score / 30;
}

static void confirm_exit(void)
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

static void parse_cmdline(int argc, char* argv[])
{
	int ch;

	while ((ch = getopt(argc, argv, "hvnd:")) != -1) {
		switch (ch) {
		case 'h': 
			puts(USAGE);
			exit(EXIT_SUCCESS);
		case 'v': 
			puts(VERSIONSTRING);
			exit(EXIT_SUCCESS);
		case 'n':
			limit = false;
			break;
		case 'd': 
			switch (optarg[0]) {
			case 'e': difficulty = EASY; break;
			case 'm': difficulty = MEDIUM; break;
			case 'h': difficulty = HARD; break;
			default: 
				puts("Invalid difficulty, allowed values: (e)asy, (m)edium, (h)ard"); 
				exit(EXIT_FAILURE);
			}
			break;
		default:
			puts(USAGE);
			exit(EXIT_FAILURE);
		}
	}
}

_Noreturn static void main_loop(void) 
{
	while (true) {
		switch (getch()) {
		case KEY_UP: 
			if (direction != DOWN || !score)
				direction = UP;
			break;
		case KEY_DOWN:
			if (direction != UP || !score)
				direction = DOWN;
			break;
		case KEY_LEFT:
			if (direction != RIGHT || !score)
				direction = LEFT;
			break;
		case KEY_RIGHT:
			if (direction != LEFT || !score)
				direction = RIGHT;
			break;
		case SPACE: 
			for (int i = 0; i < 5; i++) {
				advance();
				usleep(10000);
			}
			break;
		case 'p': 
			print_center(0, "Game paused - press 'p' to resume");
			while (getch() != 'p')
				/* wait */;
			break;
		case 'q': 
			confirm_exit();
		}
		advance();
	}
}

static void init_curses(void)
{
	initscr();
	cbreak();
	keypad(stdscr, true);
	noecho();
	curs_set(false);
}

int main(int argc, char *argv[]) 
{
	srand((unsigned) time(NULL));
	load_score();
	parse_cmdline(argc, argv);
	init_curses();	
	init_game();
	atexit(quit_game);
	main_loop();	
}
