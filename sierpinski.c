/* File: sierpinski.c
 * Date: 2019-11-02
 * 
 * Draws a Sierspinsky triangle on the terminal screen.
 * 
 * This example shows
 * - usage of the ncurses library
 * - simple line drawing algorithm
 * - recursive function calling
 * 
 * Compile and run on Linux:
 * > gcc -Wall -Wextra -std=c99 -pedantic -o sierpinski sierpinski.c -lncurses && ./sierpinski
 *
 */
#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MSG1 "Sierpinski triangle"
#define MSG2 "Hit <ENTER> to exit"

static int init()
{
	return !(initscr()                 == NULL    /* init curses */
	    || cbreak()                    == ERR     /* disable line buffering */
	    || noecho()                    == ERR     /* do our own echoing */
	    || nonl()                      == ERR     /* don't translate return key into newline */
	    || intrflush(stdscr, FALSE)    == ERR     /* prevent flush when interrupt key is pressed */
	    || keypad(stdscr, TRUE)        == ERR     /* return single value for function keys */
	);
}

static void deinit(void)
{
	endwin();
}

static void draw_line(int, int, int, int, chtype);
static void draw_sierpinski(int, int, int, int, int, int, int);

int main(int argc, char **argv)
{
	(void) argc;
	
	if (!init())
	{
		fprintf(stderr, "%s: %s\n", argv[0], "init failed.");
		return EXIT_FAILURE;
	}

    draw_sierpinski(65,  0,   0, 65, 130, 65, 4); /* Sierpinski triangle */
    draw_sierpinski(200, 0, 135, 65, 265, 65, 7); /* Sierpinski triangle with deeper recursion */

    mvaddstr(1, COLS/2-strlen(MSG1)/2, MSG1);
    mvaddstr(4, COLS/2-strlen(MSG2)/2, MSG2);

    refresh();
    getch();

	deinit();
	return EXIT_SUCCESS;
}

/* Draw a Sierpinski triangle.
 * 
 * The Sierpinski triangle is a fractal figure. It divides the sides by factor two (s=1/2).
 * The result is getting three new trinagles (N=3). The fractal dimension D therefore is
 * log(N)/log(1/s) = log(3)/log(1/(1/2)) = 1.58496...
 */
static void draw_sierpinski(int ax, int ay, int bx, int by, int cx, int cy, int depth)
{
    if (depth == 0)
        return;

    draw_sierpinski(bx+(ax-bx)/2, ay+(by-ay)/2, bx, by, ax, by, depth-1); /* left triangle */
    draw_sierpinski(ax+(cx-ax)/2, ay+(cy-ay)/2, ax, by, cx, cy, depth-1); /* right triangle */
    draw_sierpinski(ax, ay, bx+(ax-bx)/2, ay+(by-ay)/2, ax+(cx-ax)/2, ay+(cy-ay)/2, depth-1); /* upper triangle */

    draw_line(ax, ay, bx, by, ACS_DIAMOND);
    draw_line(bx, by, cx, cy, ACS_DIAMOND);
    draw_line(cx, cy, ax, ay, ACS_DIAMOND);
}

/* Connect two points ('x0', 'y0') and ('x1', 'y1') with adjacent symbols 'ch'.
 * 
 * I came up with this solution after studying some texts about line drawing
 * for digital plotter. This simple method only uses integer addition/
 * substraction and a single lower-or-equal compare operation inside a loop.
 * 
 * For the 1st octant (both x and y are incremented) we continuously increment x.
 * But we only increment y if the result of the repeated substraction of delta-y
 * from delta-x becomes <= 0. The same principle applies for the other octants.
 * 
 * If you look for an optimal algorithm then check "Bresenham's line algorithm".
 */
static void draw_line(int x0, int y0, int x1, int y1, chtype ch)
{
    if (x1 >= x0)
    {
        int dx = x1 - x0;
        if (y1 >= y0) {
            int dy = y1 - y0;
            if (dx >= dy) {
                int dec = dx;
                for ( ; x0 <= x1; x0++) {   
                    if (dec <= 0) {
                        dec += dx;
                        y0++;
                    }
                    mvaddch(y0, x0, ch);
                    dec -= dy;
                }
            } else {
                int dec = dy;
                for ( ; y0 <= y1; y0++) {   
                    if (dec <= 0) {
                        dec += dy;
                        x0++;
                    }
                    mvaddch(y0, x0, ch);
                    dec -= dx;
                }
            }
        } else {
            int dy = y0 - y1;
            if (dx >= dy) {
                int dec = dx;
                for ( ; x0 <= x1; x0++) {   
                    if (dec <= 0) {
                        dec += dx;
                        y0--;
                    }
                    mvaddch(y0, x0, ch);
                    dec -= dy;
                }
            } else {
                int dec = dy;
                for ( ; y0 >= y1; y0--) {   
                    if (dec <= 0) {
                        dec += dy;
                        x0++;
                    }
                    mvaddch(y0, x0, ch);
                    dec -= dx;
                }
            }
        }
    } else {
        int dx = x0 - x1;
        if (y1 >= y0) {
            int dy = y1 - y0;
            if (dx >= dy) {
                int dec = dx;
                for ( ; x0 >= x1; x0--) {   
                    if (dec <= 0) {
                        dec += dx;
                        y0++;
                    }
                    mvaddch(y0, x0, ch);
                    dec -= dy;
                }
            } else {
                int dec = dy;
                for ( ; y0 <= y1; y0++) {   
                    if (dec <= 0) {
                        dec += dy;
                        x0--;
                    }
                    mvaddch(y0, x0, ch);
                    dec -= dx;
                }
            }
        } else {
            int dy = y0 - y1;
            if (dx >= dy) {
                int dec = dx;
                for ( ; x0 >= x1; x0--) {   
                    if (dec <= 0) {
                        dec += dx;
                        y0--;
                    }
                    mvaddch(y0, x0, ch);
                    dec -= dy;
                }
            } else {
                int dec = dy;
                for ( ; y0 >= y1; y0--) {   
                    if (dec <= 0) {
                        dec += dy;
                        x0--;
                    }
                    mvaddch(y0, x0, ch);
                    dec -= dx;
                }
            }
        }
    }
}
