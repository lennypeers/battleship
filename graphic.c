#include "variables.h"

#define GRID_H 21
#define GRID_W 41

#define STATUS_H 10
#define STATUS_W 12

#define WIN_MARGIN ((COLS - 2 * (GRID_W + STATUS_W) ) / 5)

void
draw_grid(Panel panel)
{
	int i, j;
	box(panel.grid, 0, 0);

	for (i = 0; i < 19; ++i) {
		if (i % 2 == 0)	/* vertical seps */
			for (j = 1; j <= 9; ++j)
				mvwaddch(panel.grid, i + 1, 4 * j, ACS_VLINE);
		else	/* intersections and horizontal seps */
			for (j = 1; j <= 10; ++j) {
				if (j != 10)
					mvwaddch(panel.grid, i + 1, 4 * j, ACS_PLUS);
				mvwaddch(panel.grid, i + 1, 4 * j - 1, ACS_HLINE);
				mvwaddch(panel.grid, i + 1, 4 * j - 2, ACS_HLINE);
				mvwaddch(panel.grid, i + 1, 4 * j - 3, ACS_HLINE);
			}
	}
	wrefresh(panel.grid);
}

void
draw_status(Panel panel)
{
	int line;
	char message[6][STATUS_W] = { 	"1x1",
									"1x2",
									"1x3",
									"1x4",
									"2x1",
									"3x1",
								};

	box(panel.status, 0, 0);
	mvwprintw(panel.status,  1, 1, "left (HxW)" );
	mvwhline(panel.status, 2, 1, ACS_HLINE, STATUS_W - 2);

	for (line = 0; line < 6; ++line)
		mvwprintw(panel.status, line + 3, 1, "  %s: %d", message[line], panel.val_status[line] );

	wrefresh(panel.status);
}

void
draw_all_status(Screen screen)
{
	draw_status(screen.left);
	draw_status(screen.right);
}

void
draw_screen(Screen screen)
{
	draw_grid(screen.left);
	draw_grid(screen.right);
	draw_status(screen.left);
	draw_status(screen.right);
}

void
colorize_grid(Panel panel)
{
	int i, j, color;
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_BLUE, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	init_pair(5, COLOR_BLACK, COLOR_BLACK);
	init_pair(6, COLOR_CYAN, COLOR_BLACK);

	for (i = 0; i < 10; ++i)
		for (j = 0; j < 10; ++j) {
			switch (panel.val_grid[i][j]) {
			case SEA:
				wattron(panel.grid, COLOR_PAIR(1));
				mvwaddch(panel.grid, 2 * i + 1, 4 * j + 2, ACS_DIAMOND);
				wattroff(panel.grid, COLOR_PAIR(1));
				continue;
			case SUNKED:
				color = 2;
				break;
			case TOUCHED:
				color = 3;
				break;
			case POPULATING:
				color = 4;
				break;
			case REVEAL:
				color = 6;
				break;
			default:
				color = 5;
				break;
			}
				wattron(panel.grid, COLOR_PAIR(color));
				mvwaddch(panel.grid, 2 * i + 1, 4 * j + 1, ACS_CKBOARD);
				mvwaddch(panel.grid, 2 * i + 1, 4 * j + 2, ACS_CKBOARD);
				mvwaddch(panel.grid, 2 * i + 1, 4 * j + 3, ACS_CKBOARD);
				wattroff(panel.grid, COLOR_PAIR(color));
		}
	wrefresh(panel.grid);
}

void
colorize_screen(Screen screen)
{
	colorize_grid(screen.left);
	colorize_grid(screen.right);
}

static void
move_target(Point coord, Panel panel)
{
	int i;
	draw_grid(panel);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	wattron(panel.grid, COLOR_PAIR(1));

	/* top and bottom lines */
	for (i = 0; i < 2; ++i) {
		mvwaddch(panel.grid, 2 * coord.x + 2 * i, 4 * coord.y + 1, ACS_HLINE);
		mvwaddch(panel.grid, 2 * coord.x + 2 * i, 4 * coord.y + 2, ACS_HLINE);
		mvwaddch(panel.grid, 2 * coord.x + 2 * i, 4 * coord.y + 3, ACS_HLINE);
	}
	/* left, right lines */
	mvwaddch(panel.grid, 2 * coord.x + 1, 4 * coord.y, ACS_VLINE);
	mvwaddch(panel.grid, 2 * coord.x + 1, 4 * coord.y + 4, ACS_VLINE);
	/* four corners */
	mvwaddch(panel.grid, 2 * coord.x, 4 * coord.y, ACS_ULCORNER);
	mvwaddch(panel.grid, 2 * coord.x, 4 * coord.y + 4, ACS_URCORNER);
	mvwaddch(panel.grid, 2 * coord.x + 2, 4 * coord.y + 4, ACS_LRCORNER);
	mvwaddch(panel.grid, 2 * coord.x + 2, 4 * coord.y, ACS_LLCORNER);

	wattroff(panel.grid, COLOR_PAIR(1));
	wrefresh(panel.grid);
}

void
choose_target(Panel panel, Point *point)
{
	int ch;
	move_target(*point, panel);

	while ((ch = getch()) != 10 && ch != 9) {
		switch (ch) {
		case KEY_LEFT: case 'a':
			if (point->y > 0)
				--(point->y);
			break;
		case KEY_RIGHT: case 'd':
			if (point->y < 9)
				++(point->y);
			break;
		case KEY_UP: case 'w':
			if (point->x > 0)
				--(point->x);
			break;
		case KEY_DOWN: case 's':
			if (point->x < 9)
				++(point->x);
			break;
		}
		move_target(*point, panel);
	}
}

static Panel
create_panel(int side)
{
	int GRID_Y, GRID_X, STATUS_Y, STATUS_X;
	Panel panel = { .val_status = {4, 1, 1, 1, 2, 1},
					.val_grid = {[0 ... 9][0 ... 9] = NOTHING_HIDDEN} };

	GRID_Y = (LINES - GRID_H) / 2;
	STATUS_Y = (LINES - STATUS_H) / 2;

	if (side == LEFT_PLAYER) {
		GRID_X = WIN_MARGIN;
		STATUS_X = 2 * WIN_MARGIN + GRID_W;
	} else {
		GRID_X = 3 * WIN_MARGIN + GRID_W + STATUS_W;
		STATUS_X = 4 * WIN_MARGIN + 2 * GRID_W + STATUS_W;
	}

	panel.grid = newwin(GRID_H, GRID_W, GRID_Y, GRID_X);
	panel.status = newwin(STATUS_H, STATUS_W, STATUS_Y, STATUS_X);

	return(panel);
}

void
init_screen(Screen *screen)
{
	screen->left = create_panel(LEFT_PLAYER),
	screen->right = create_panel(RIGHT_PLAYER),
	screen->player = LEFT_PLAYER;
}

/* greetings */

#define N_CHOICES 3

static char
*choices[] = {
	"One player mode",
	"Two player mode",
	"Exit",
};

static void
print_menu(WINDOW *menu_win, int highlight)
{
	int x, y, i;

	x = 1;
	y = 1;
	box(menu_win, 0, 0);
	for(i = 0; i < N_CHOICES; ++i)
	{	if(highlight == i + 1) /* High light the present choice */
		{	wattron(menu_win, A_REVERSE);
			mvwprintw(menu_win, y, x, "%s", choices[i]);
			wattroff(menu_win, A_REVERSE);
		}
		else
			mvwprintw(menu_win, y, x, "%s", choices[i]);
		++y;
	}
	wrefresh(menu_win);
}

int
greetings(void)
{
	int dialog_H = 5;
	int dialog_W = 17;
	int startx = (COLS - dialog_W) / 2;
	int starty = (LINES - dialog_H) / 2;

	WINDOW *menu_win;
	int highlight = 1;
	int choice = 0;
	int c;

	menu_win = newwin(dialog_H, dialog_W, starty, startx);
	keypad(menu_win, TRUE);
	refresh();
	print_menu(menu_win, highlight);
	while(1)
	{	c = wgetch(menu_win);
		switch(c)
		{	case KEY_UP:
				if(highlight == 1)
					highlight = N_CHOICES;
				else
					--highlight;
				break;
			case KEY_DOWN:
				if(highlight == N_CHOICES)
					highlight = 1;
				else
					++highlight;
				break;
			case 10:
				choice = highlight;
				break;
			default:
				break;
		}
		print_menu(menu_win, highlight);
		if(choice != 0)	/* User did a choice come out of the infinite loop */
			break;
	}
	clrtoeol();
	refresh();
	endwin();
	return(choice);
}
