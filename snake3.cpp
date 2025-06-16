#include "sclass.h"
#include <ctime>
#include <unistd.h>

time_t gameStartTime;
time_t gameOverTime;
time_t speedIncreaseTime;

using std::fstream;
using std::ifstream;
using std::ofstream;

bool writeEndAndGetInput();
void printScore(WINDOW *, int, int, int);
void printMission(WINDOW *, int, int);
void draw(WINDOW *, SnakeGame &, char *, int, int);
void processInput(WINDOW *, SnakeGame &, int);
int MAP(int x, int y);

int main()
{
	appleCount = 0;
	poisonCount = 0;
	gateCount = 0;
	slowItemCount = 0; // 새로운 아이템 카운트 초기화
	appleMissionComplete = ' ';
	maxSizeMissionComplete = ' ';
	gateMissionComplete = ' ';
	poisonMissionComplete = ' ';
	slowItemMissionComplete = ' '; // 새로운 아이템 미션 완료 여부 초기화
	int sz;
	int x, y;
	x = 100;
	y = 30;
	initscr();
	noecho();
	cbreak();
	start_color();
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	int coin = 0;
	mvprintw(y / 2, x / 2 - 10, "Insert coin to play game.");
	mvprintw(y / 2 + 1, x / 2 - 10, "Press button to insert a coin.");
	refresh();
	getch();
	endwin();

	WINDOW *win = newwin(y + 2, x - 38, 0, 0);

	wbkgd(win, COLOR_PAIR(1));
	wattron(win, COLOR_PAIR(1));
	wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');

	nodelay(win, TRUE);
	keypad(win, TRUE);

	WINDOW *score = newwin(y / 2, x - 60, 0, x - 37);
	wbkgd(score, COLOR_PAIR(1));
	wattron(score, COLOR_PAIR(1));
	wborder(score, '|', '|', '-', '-', '+', '+', '+', '+');
	wrefresh(score);
	nodelay(score, TRUE);

	WINDOW *mission = newwin(y / 2, x - 60, y / 2, x - 37);
	wbkgd(mission, COLOR_PAIR(1));
	wattron(mission, COLOR_PAIR(1));
	wborder(mission, '|', '|', '-', '-', '+', '+', '+', '+');
	wrefresh(mission);
	nodelay(mission, TRUE);

	do
	{
		gameStartTime = time(NULL);
		speedIncreaseTime = gameStartTime + 5; // 5초 후에 속도 증가
		SnakeGame snake(y, x - 40);
		snake.resetSpeed();
		refresh();
		wrefresh(win);
		sz = snake.getBodySize();
		printScore(score, 0, 1, sz);
		printMission(mission, 0, 1);
		appleCount = 0;
		poisonCount = 0;
		gateCount = 0;
		slowItemCount = 0; // 새로운 아이템 카운트 초기화

		while (!snake.getExit())
		{
			char *tbl = snake.getMap();
			draw(win, snake, tbl, snake.getHeight(), snake.getWidth());

			printScore(score, snake.getPoints(), snake.getLevel(), snake.getBodySize());
			printMission(mission, snake.getPoints(), snake.getLevel());

			int input = wgetch(win);
			processInput(win, snake, input);
			if (snake.getExit())
				break;
			snake.checkPoints();
			snake.moveBody();
			snake.move();
			snake.checkForApple();
			snake.checkForPoison();
			snake.checkForSlowItem(); // 새로운 아이템 체크
			snake.checkWalls();
			snake.checkGate();
			snake.changeGate();
			snake.updateMissionStatus();
			snake.getSize();

			if (time(NULL) >= speedIncreaseTime && !snake.isSlowActive())
			{
				snake.increaseSpeed();
				speedIncreaseTime = time(NULL) + 5;
			}

#ifdef WIN32
#else
			usleep(snake.getSpeed());
#endif
		}

	} while (writeEndAndGetInput());

	delwin(score);
	delwin(win);
	delwin(mission);
	endwin();
}

bool writeEndAndGetInput()
{
	gameOverTime = time(NULL);
	int score = appleCount + maxSizeCount - poisonCount + gateCount * 2;
	WINDOW *endwin = newwin(32, 110, 0, 0);
	box(endwin, 0, 0);
	nodelay(endwin, TRUE);
	keypad(endwin, TRUE);
	mvwprintw(endwin, 14, 40, "GAME OVER");
	mvwprintw(endwin, 15, 40, "Game Score : %d", score);
	mvwprintw(endwin, 16, 40, "Play Time : %d second", gameOverTime - gameStartTime);
	mvwprintw(endwin, 17, 40, "Total Score : %d", score + 1000 - (gameOverTime - gameStartTime));
	mvwprintw(endwin, 18, 40, "Press [q] to quit.");
	int c;
	do
	{
		c = wgetch(endwin);
	} while (c != 10 && c != ' ' && c != 'q' && c != 'Q');
	werase(endwin);
	wrefresh(endwin);
	delwin(endwin);
	return (c == 'q' || c == 'Q') ? false : true;
}

void printScore(WINDOW *w, int score, int level, int sz)
{
	werase(w);
	wbkgd(w, COLOR_PAIR(1));
	wattron(w, COLOR_PAIR(1));
	wborder(w, '|', '|', '-', '-', '+', '+', '+', '+');
	mvwprintw(w, 1, 2, "Score Board");
	mvwprintw(w, 4, 2, "B: Current Length / Max Length %d/%d", sz, maxSizeMission);
	mvwprintw(w, 6, 2, "+: Number of Growth Items %d", appleCount);
	mvwprintw(w, 8, 2, "-: Number of Poison Items %d", poisonCount);
	mvwprintw(w, 10, 2, "G: Number of gate used %d", gateCount);
	wrefresh(w);
}

void printMission(WINDOW *w, int score, int level)
{
	werase(w);
	wbkgd(w, COLOR_PAIR(1));
	wattron(w, COLOR_PAIR(1));
	wborder(w, '|', '|', '-', '-', '+', '+', '+', '+');
	mvwprintw(w, 1, 2, "Mission Board");
	mvwprintw(w, 4, 2, "B: %d   (%c)", maxSizeMission, maxSizeMissionComplete);
	mvwprintw(w, 5, 2, "+: %d   (%c)", appleMission, appleMissionComplete);
	mvwprintw(w, 6, 2, "-: %d   (%c)", poisonMission, poisonMissionComplete);
	mvwprintw(w, 7, 2, "G: %d   (%c)", gateMission, gateMissionComplete);
	mvwprintw(w, 8, 2, "S: %d   (%c)", slowItemMission, slowItemMissionComplete); // 새로운 아이템 미션
	wrefresh(w);
}

void draw(WINDOW *win, SnakeGame &snake, char *table, int height, int width)
{
	werase(win);
	wbkgd(win, COLOR_PAIR(1));
	wattron(win, COLOR_PAIR(1));
	wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
	int ch;

	for (int i = 0; i < (height * width); ++i)
	{
		if (table[i] != ' ')
		{
			int y = i / width;
			int x = i - (y * width);

			int d;
			switch (table[i])
			{
			case 'a':
				ch = '*';
				break;
			case 'x':
				ch = 'x';
				break;
			case 'h':
				ch = 'O';
				break;
			case 'b':
				ch = 'o';
				break;
			case '1':
				ch = 'a';
				break;
			case '2':
				ch = 'b';
				break;
			case '3':
				ch = ' ';
				break;
			case 'G':
				ch = ' ';
				break;
			case 's':
				ch = 'S';
				break;
			}
			mvwaddch(win, 1 + y, 1 + x, ch);
		}
	}
	wrefresh(win);
}

void processInput(WINDOW *win, SnakeGame &snake, int input)
{
	int d = snake.getDirection();
	switch (input)
	{
	case KEY_UP:
		if (d == 2)
		{
			snake.setExit(true);
			break;
		}
		if (d != 0 && !(d == 2 && snake.getPoints() > 0))
			snake.setDirection(0);
		break;
	case KEY_DOWN:
		if (d == 0)
		{
			snake.setExit(true);
			break;
		}
		if (d != 2 && !(d == 0 && snake.getPoints() > 0))
			snake.setDirection(2);
		break;
	case KEY_LEFT:
		if (d == 1)
		{
			snake.setExit(true);
			break;
		}
		if (d != 3 && !(d == 1 && snake.getPoints() > 0))
			snake.setDirection(3);
		break;
	case KEY_RIGHT:
		if (d == 3)
		{
			snake.setExit(true);
			break;
		}
		if (d != 1 && !(d == 3 && snake.getPoints() > 0))
			snake.setDirection(1);
		break;
	case 'Q':
	case 'q':
		snake.setExit(true);
		break;
	case 'P':
	case 'p':
		char c;
		wattron(win, A_BOLD);
		mvwprintw(win, snake.getHeight() / 2, snake.getWidth() / 2, "PAUSE");
		wattroff(win, A_BOLD);
		nodelay(win, FALSE);
		do
		{
			c = wgetch(win);
		} while (c != 'p' && c != 'P');
		nodelay(win, TRUE);
		break;
	}
}
