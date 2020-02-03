/* File: colorscroll.c
 * Date: 2019-11-04
 *
 * Modifying the color palette to create a simple scrolling effect.
 *
 * This example shows
 * - colored output
 * - simple animation
 *
 * Compile and run on Linux:
 * > gcc -Wall -Wextra -std=c99 -pedantic -o colorscroll colorscroll.c -lncurses && ./colorscroll
 *
 */
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>

#define PAL_COLOR_INDEX(i) ((i)+8)
#define PAL_PAIR_INDEX(i) ((i)+1)
#define PAL_FADE_IN   1
#define PAL_FADE_OUT -1
#define PAL_NUM_COLORS 128

struct palette
{
    int num;
    short start;
    short max;
    short step;
    signed char fade[PAL_NUM_COLORS];
};

static bool initUI();
static void deinitUI();
static bool init_palette(struct palette *);
static bool update_palette(struct palette *);
static void draw_rect(int, int, int, int);

int main(__attribute__((unused)) int argc, __attribute__((unused)) char **argv)
{
    struct palette palette;
    int ret = EXIT_SUCCESS;

    if (!initUI())
        return EXIT_FAILURE;

    if (!init_palette(&palette))
    {
        deinitUI();
        return EXIT_FAILURE;
    }

    /* render loop */
    while(true)
    {
        int x, y;

        /* logic */
        if (!update_palette(&palette))
        {
            ret = EXIT_FAILURE;
            break;
        }

        /* drawing */
        for (x = 0; x < palette.num; x++)
            for (y = 0; y < 50; y++)
                mvaddch(y, x, ACS_CKBOARD | COLOR_PAIR(PAL_PAIR_INDEX(x)));
        draw_rect(0, 0, palette.num, 50);
        mvaddstr(0, 4, " Press ESC to exit. ");

        /* flip to screen */
        if (refresh() == ERR)
        {
            ret = EXIT_FAILURE;
            break;
        }

        /* input */
        if (getch() == 0x1b)
            break;

        /* pause the process */
        napms(30);
    }

    deinitUI();
    return ret;
}

static bool initUI()
{
    return !(initscr()                   == NULL   /* init curses */
             || has_colors()             == FALSE  /* terminal can manipulate colors */
             || can_change_color()       == FALSE  /* terminal can change color definitions */
             || start_color()            == ERR    /* use color routines */
             || (COLORS < 256)           == TRUE   /* our program requires multiple colors */
             || (COLOR_PAIRS < 256)      == TRUE   /* our program requires multiple color pairs */
             || cbreak()                 == ERR    /* disable line buffering */
             || noecho()                 == ERR    /* do our own echoing */
             || nonl()                   == ERR    /* don't translate return key into newline */
             || intrflush(stdscr, FALSE) == ERR    /* prevent flush when interrupt key is pressed */
             || keypad(stdscr, TRUE)     == ERR    /* return single value for function keys */
             || nodelay(stdscr, TRUE)    == ERR    /* non-blocking read */
             || (!getenv("ESCDELAY") && set_escdelay(0) == ERR) /* turn off ESC delay if not set from outside */
    );
}

static void deinitUI()
{
    endwin();
}

/* initialize our color palette */
static bool init_palette(struct palette *palette)
{
    short i, b;

    palette->num   = PAL_NUM_COLORS;
    palette->max   = 1000;
    palette->start = 50;

    b = palette->start;

    palette->step = (palette->max - palette->start) / (palette->num >> 1);

    for (i = 0; i < palette->num; i++)
    {
        if (init_color(PAL_COLOR_INDEX(i), 0, 0, b) == ERR)
            return false;
        if (init_pair(PAL_PAIR_INDEX(i), PAL_COLOR_INDEX(i), COLOR_BLACK) == ERR)
            return false;
        if (i < palette->num >> 1)
        {
            b += palette->step;
            palette->fade[i] = PAL_FADE_IN;
        }
        else
        {
            b -= palette->step;
            palette->fade[i] = PAL_FADE_OUT;
        }
    }
    return true;
}

/* changing the color values creates a simple scrolling effect */
static bool update_palette(struct palette *palette)
{
    short i;
    for (i = 0; i < palette->num; i++)
    {
        short r = 0, g = 0, b = 0;
        if (color_content(PAL_COLOR_INDEX(i), &r, &g, &b) == ERR)
            return false;

        if (b == palette->start + (palette->step * palette->num >> 1))
            palette->fade[i] = PAL_FADE_OUT;
        else if(b == palette->start)
            palette->fade[i] = PAL_FADE_IN;

        if (init_color(PAL_COLOR_INDEX(i), r, g, b + palette->step * palette->fade[i]) == ERR)
            return false;
    }
    return true;
}

/* Draws a rectangle. */
static void draw_rect(int x, int y, int length, int height)
{
    int i;

    if (length <= 0 || height <= 0)
        return;

    for (i = x; i < x + length; i++)
    {
        mvaddch(y, i, ACS_HLINE);
        mvaddch(y + height - 1, i, ACS_HLINE);
    }

    for (i = y; i < y + height; i++)
    {
        mvaddch(i, x, ACS_VLINE);
        mvaddch(i, x + length - 1, ACS_VLINE);
    }

    mvaddch(y,              x,              ACS_ULCORNER);
    mvaddch(y + height - 1, x,              ACS_LLCORNER);
    mvaddch(y,              x + length - 1, ACS_URCORNER);
    mvaddch(y + height -1,  x + length - 1, ACS_LRCORNER);
}
