/* 
 * Snake game
 * Copyright (c) 2017 Alessandro Righi - MIT license
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <ncurses.h> 
#include <unistd.h>
#include <signal.h>

#define VERSION "1.1.1"
#define VERSIONSTRING "Snake v" VERSION " (c) 2017 Alessandro Righi"
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define SNAKE_HEAD '@'
#define SNAKE_BODY '#'
#define POWERUP '$'
#define SUPER_POWERUP '%'
#define BOMB '*'
#define SPACE ' '
#define CORNER '+'
#define EDGE_VERTICAL '|'
#define EDGE_ORIZONTAL '-'
#define MAX_POS 1024
#define SCREEN ((char(*)[X]) screen)
#define PRINT_CENTER(i, message, ...) mvprintw(Y/2+i, X/2 - strlen(message)/2, message, ##__VA_ARGS__)
#define EASY 90000
#define MEDIUM 60000
#define HARD 30000

static int direction;
static int length;
static int (*screen)[];
static int Y;
static int X;
static int level;
static int score;
static int high_score;
static unsigned int head_pos;
static unsigned int tail_pos;
static const char *SCORE_FILENAME = ".snake_score";
static struct point { int x, y; } position[MAX_POS];
static int difficulty = MEDIUM;

void load_score() 
{
	char filename[1024];
	FILE *score_fp;
	
	sprintf(filename, "%s/%s", getenv("HOME"), SCORE_FILENAME);
	if ((score_fp = fopen(filename, "r"))) {
		fscanf(score_fp, "%d", &high_score);
		fclose(score_fp);
	} 
}

void save_score() 
{
	char filename[1024];
	FILE *score_fp;

	sprintf(filename, "%s/%s", getenv("HOME"), SCORE_FILENAME);
	if ((score_fp = fopen(filename, "w+"))) {
		fprintf(score_fp, "%d\n", high_score);
		fclose(score_fp);
	} 
}

void add_powerup() 
{
	int x, y;
	do {
		x = rand() % COLS;
		y = rand() % LINES;
	} while (SCREEN[y][x] != SPACE);

	switch (rand() % 10) {
	case 7:
		SCREEN[y][x] = SUPER_POWERUP;	
		break;
	case 3:
		SCREEN[y][x] = BOMB;
		add_powerup();
		break;
	default: 
		SCREEN[y][x] = POWERUP;
	}
}

void remove_tail() 
{
	SCREEN[position[tail_pos].y][position[tail_pos].x] = SPACE;
	tail_pos = (tail_pos + 1) % MAX_POS;
}

void add_head(struct point head) 
{
	SCREEN[position[head_pos].y][position[head_pos].x] = SNAKE_BODY;
	SCREEN[head.y][head.x] = SNAKE_HEAD;	
	head_pos = (head_pos + 1) % MAX_POS;
	position[head_pos].y = head.y;
	position[head_pos].x = head.x;
}

void print_screen() 
{
	clear();
	for (int i = 0; i < Y; i++)
		for (int j = 0; j < X; j++)
			addch(SCREEN[i][j]);
	refresh();
}

void dup_tail() 
{
	struct point tail = position[tail_pos];
	tail_pos = (tail_pos - 1) % MAX_POS;
	position[tail_pos] = tail;
	length++;
	score++;
}

void init_game() 
{
	Y = LINES;
	X = COLS;
	length = 0;
	head_pos = 0;
	tail_pos = 0;
	score = 0;
	direction = RIGHT;
	
	screen = malloc(sizeof(char[Y][X]));
	memset(screen, SPACE, Y*X);

	for (int i = 0; i < X; i++) {
		SCREEN[0][i] = EDGE_ORIZONTAL;
		SCREEN[Y-1][i] = EDGE_ORIZONTAL;		
	}

	for (int i = 0; i < Y; i++) {
		SCREEN[i][0] = EDGE_VERTICAL;
		SCREEN[i][X-1] = EDGE_VERTICAL;		
	}

	SCREEN[0][0] = CORNER;
	SCREEN[0][X-1] = CORNER;
	SCREEN[Y-1][0] = CORNER;
	SCREEN[Y-1][X-1] = CORNER;
	
	for (head_pos = 0; head_pos < 7; head_pos++) {
		position[head_pos].x = X/2;
		position[head_pos].y = Y/2;
		SCREEN[position[head_pos].y][position[head_pos].x] = SNAKE_BODY;				
		length++;
	}
	head_pos--;
	SCREEN[position[head_pos].y][position[head_pos].x] = SNAKE_HEAD;		
	add_powerup();
	add_powerup();
	print_screen();	
}

void _Noreturn quit_game() 
{
	endwin();
	printf("Game ended\n");
	printf("You scored %d - high score is %d\n", score, high_score);
	exit(EXIT_SUCCESS);
}

void game_lost() 
{
	PRINT_CENTER(-1, "You lose!");
	if (score > high_score) {
		high_score = score;
		save_score();
		PRINT_CENTER(0, "Congratulations! New High Score %d!", score);
	} else {
		PRINT_CENTER(0, "Your score: %d", score);
	}
	PRINT_CENTER(1, "Play new game ? (y/n)");
	refresh();
	char c; 
	while ((c = getch())) {
		switch (c) {
		case 'y':
			init_game();
			return;
		case 'n': 
			quit_game();
		}
	}
}

void advance()
{
	if (!score)
		score = 1;
	if (direction == LEFT || direction == RIGHT)
		ualarm(difficulty-level*3000,0);	
	else
		ualarm(difficulty*2-level*6000,0);	

	struct point head = position[head_pos];

	switch (direction) {
	case UP: 
		head.y -= 1;
		break;
	case DOWN: 
		head.y += 1;
		break;
	case LEFT: 
		head.x -= 1;
		break;
	case RIGHT:
		head.x += 1;
		break;
	}

	switch (SCREEN[head.y][head.x]) {
	case SPACE:
		add_head(head);
		remove_tail();		
		break;		
	case SUPER_POWERUP:
		for (int i = 0; i < 15; i++)
			dup_tail();
	case POWERUP:
		add_head(head);
		add_powerup();
		length++;
		score++;		
		break;
	case BOMB:
		add_head(head);
		remove_tail();
		for (int i = 0; i < 25; i++) {
			remove_tail();
			if (--length == 0)
				break;
		}
		if (length)
			break;
	default: 
		alarm(0);
		add_head(head);
		remove_tail();
		for (int i = 0; i < 5; i++) {
			SCREEN[head.y][head.x] = SUPER_POWERUP;		
			print_screen();			
			usleep(140000);	
			SCREEN[head.y][head.x] = SNAKE_HEAD;
			print_screen();
			usleep(140000);		
		}
		game_lost();
	}
	print_screen();
	level = score / 30;
}

void _Noreturn usage()
{
	printf("Snake version " VERSION "\n"
		   "Usage: snake [-hvd]\n"
		   "    -d {easy/medium/hard} select difficulty\n"
		   "    -h                    show this help\n"
		   "    -v                    version string\n"
		   " * move the snake with arrow keys or wasd\n"
		   " * space bar for extra speed (use with caution)\n"
		   " * q for quitting the game\n"
		   " * p pause the game\n"
		   " * if you hit the borders of the screen, or your tail, you lose\n"		   
		   " * powerups:\n"
		   "     $ - increase snake length by 1\n"
		   "     %% - increase snake length by 15\n"
		   "     * - decrease snake length by 25\n"
		);
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) 
{
	char c;
	while ((c = getopt(argc, argv, "hvd:")) != -1) {
		switch (c) {
		case 'h': 
			usage();
		case 'v': 
			puts(VERSIONSTRING);
			exit(EXIT_SUCCESS);
		case 'd': 
			if (!strcmp(optarg, "easy"))
				difficulty = EASY;
			else if (!strcmp(optarg, "medium"))
				difficulty = MEDIUM;
			else if (!strcmp(optarg, "hard"))
				difficulty = HARD;
			else 
				usage();
			break;
		default:
			usage();
		}
	}

	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);
	srand(time(NULL));
	signal(SIGALRM, advance);
	load_score();
	init_game();

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
			PRINT_CENTER(0, "Game paused - press 'p' to resume");
			alarm(0);
			while (getch() != 'p')
				/* wait */;
			break;
		case 'q': 
			quit_game();
		}
		advance();
	}
	return 0;
}
