/* File: starfield.c
 * Date: 2019-11-06
 *
 * A moving star field.
 *
 * This example shows
 * - control of the animation speed with the frames-per-second count
 * - animation loop
 *
 * Compile and run on Linux:
 * > gcc -Wall -Wextra -std=c99 -pedantic -o starfield starfield.c -lncurses && ./starfield
 *
 */
#define _GNU_SOURCE /* clock functions */
#include <time.h>
#include <stdlib.h>
#include <ncurses.h>
#include <stdbool.h>

#define PIXEL_LAYERS 3    /* three layers of stars */
#define PIXEL_COUNT  128
#define PIXEL_MAX_X  COLS
#define PIXEL_MAX_Y  80
#define PIXEL_GRAY1  1    /* color pair index for custom colors */
#define PIXEL_GRAY2  2
#define PIXEL_GRAY3  3

/* Number of frames-per-second to aim for. Based on this value we calculate the
 * time delay so that our animation speed is the same on slow and fast PCs.
 * It's important to not mix up the FPS count with the velocity of the animated
 * objects. The objects are controlled by the update functions. */
static const int FPS = 30;

struct pixels
{
    struct
    {
        int x;
        int y;
    } coord[PIXEL_LAYERS][PIXEL_COUNT];
    int speed[PIXEL_LAYERS];
    int color[PIXEL_LAYERS];
    int count[PIXEL_LAYERS];
};

static bool initUI();
static void deinitUI();
static bool init_colors();
static void init_pixels(struct pixels *);
static void update_pixels(struct pixels *);
static bool draw_pixels(const struct pixels *);
static long adjust_delay(long, const struct timespec *);

int main(__attribute__((unused)) int argc, __attribute__((unused)) char **argv)
{
    struct pixels *pixels;
    int frames = 0;
    long delay = 0;
    struct timespec start_time;
    int ret = EXIT_SUCCESS;

    if ((pixels = (struct pixels *) malloc(sizeof(struct pixels))) == NULL)
        return EXIT_FAILURE;

    if (!initUI())
    {
        free(pixels);
        return EXIT_FAILURE;
    }

    if (!init_colors())
    {
        deinitUI();
        free(pixels);
        return EXIT_FAILURE;
    }

    init_pixels(pixels);

    clock_gettime(CLOCK_REALTIME, &start_time); /* record the start time */

    /* main animation loop */
    while(true)
    {
        update_pixels(pixels);

        if (!draw_pixels(pixels))
        {
            ret = EXIT_FAILURE;
            break;
        }
        mvaddstr(0, 0, "Press 'q' to exit.");

        if (refresh() == ERR)
        {
            ret = EXIT_FAILURE;
            break;
        }

        if (getch() == 'q')
            break;

        if (delay > 0)
            napms(delay); /* Zzz */

        if (++frames == FPS)
        {
            /* we have rendered FPS x frames, it's time to check if we have to
             * adapt the delay time */
            frames = 0;
            delay = adjust_delay(delay, &start_time);
            clock_gettime(CLOCK_REALTIME, &start_time); /* create reference timestamp */
        }
    }

    /* clean up */
    free(pixels);
    deinitUI();
    return ret;
}

static bool initUI()
{
    return !(initscr()                 == NULL   /* init curses */
             || has_colors()                == FALSE  /* terminal can manipulate colors */
             || can_change_color()          == FALSE  /* terminal can change color definitions */
             || start_color()               == ERR    /* use color routines */
             || (COLORS < 256)              == TRUE   /* our program requires multiple colors */
             || (COLOR_PAIRS < 256)         == TRUE   /* our program requires multiple color pairs */
             || cbreak()                    == ERR    /* disable line buffering */
             || noecho()                    == ERR    /* do our own echoing */
             || nonl()                      == ERR    /* don't translate return key into newline */
             || intrflush(stdscr, FALSE)    == ERR    /* prevent flush when interrupt key is pressed */
             || keypad(stdscr, TRUE)        == ERR    /* return single value for function keys */
             || nodelay(stdscr, TRUE)       == ERR    /* non-blocking read */
             || curs_set(0)                 == ERR    /* set the cursor state to invisible */
            );
}

static void deinitUI()
{
    endwin();
}

static bool init_colors()
{
    return init_color(10,   50,   50,   50) == OK
           && init_color(11,  350,  350,  350) == OK
           && init_color(12, 1000, 1000, 1000) == OK
           && init_pair(PIXEL_GRAY1, 10, 0) == OK
           && init_pair(PIXEL_GRAY2, 11, 0) == OK
           && init_pair(PIXEL_GRAY3, 12, 0) == OK;
}

static void init_pixels(struct pixels *pixels)
{
    int i, k;

    srand(time(NULL));

    for (i = 0, k = 1; i < PIXEL_LAYERS; i++, k <<= 1)
    {
        int j;
        int n = PIXEL_COUNT / k;

        for (j = 0 ; j < n; j++)
        {
            pixels->coord[i][j].x = rand() % PIXEL_MAX_X;
            pixels->coord[i][j].y = rand() % PIXEL_MAX_Y;
        }

        pixels->speed[i] = i+1;
        pixels->count[i] = n;
    }

    pixels->color[0] = PIXEL_GRAY1;
    pixels->color[1] = PIXEL_GRAY2;
    pixels->color[2] = PIXEL_GRAY3;
}

static void update_pixels(struct pixels *pixels)
{
    int i, j;

    for (i = 0; i < PIXEL_LAYERS; i++)
    {
        for (j = 0; j < PIXEL_COUNT; j++)
        {
            if (pixels->coord[i][j].x + pixels->speed[i] >= PIXEL_MAX_X)
            {
                pixels->coord[i][j].x = 0;
                pixels->coord[i][j].y = rand() % PIXEL_MAX_Y;
            }
            else
            {
                pixels->coord[i][j].x += pixels->speed[i];
            }
        }
    }
}

static bool draw_pixels(const struct pixels *pixels)
{
    int i, j;

    if (erase() == ERR)
        return false;

    for (i = 0; i < PIXEL_LAYERS; i++)
        for (j = 0; j < pixels->count[i]; j++)
            mvaddch(pixels->coord[i][j].y, pixels->coord[i][j].x, ACS_DIAMOND | COLOR_PAIR(pixels->color[i]));

    return true;
}

static long adjust_delay(long delay, const struct timespec *start_time)
{
    long elapsed;
    struct timespec end_time;
    clock_gettime(CLOCK_REALTIME, &end_time);
    int smooth = 25;

    elapsed = (end_time.tv_sec - start_time->tv_sec) * 1000 +
              (end_time.tv_nsec - start_time->tv_nsec) / 1000000;

    if (elapsed < 1000 - smooth)
    {
        /* converge to the maximum delay time defined by the FPS constant */
        delay = (delay + 1000 / FPS) / 2;
    }
    else if (elapsed > 1000 + smooth)
    {
        /* if we were too slow, reduce the delay time */
        if (delay > 0)
            delay -= 1;
    }

    return delay;
}
