/*
 *      TTY-CLOCK Main file.
 *      Copyright © 2008 Martin Duquesnoy <xorg62@gmail.com>
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are
 *      met:
 *
 *      * Redistributions of source code must retain the above copyright *
 * notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following disclaimer
 *        in the documentation and/or other materials provided with the
 *        distribution.
 *      * Neither the name of the  nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>

#ifdef TOOT
#include <toot.h>
#endif

#include "ttytimer.h"

static bool time_is_zero(void) {
  return ttyclock->date.hour[0] == 0 && ttyclock->date.hour[1] == 0 &&
         ttyclock->date.minute[0] == 0 && ttyclock->date.minute[1] == 0 &&
         ttyclock->date.second[0] == 0 && ttyclock->date.second[1] == 0;
}

/* Prints usage message and exits with exit code exit_code. */
static void usage(char *argv0, int exit_code) {
  printf("usage : %s [-xbvih] [-C color] hh:mm:ss\n"
         "    no parameter      Stopwatch mode (count up)\n"
         "        -t            Timer: difference between given time.\n"
         "        -x            Show box\n"
         "        -d            Show starting time\n"
         "        -C color      Set the clock color\n"
         "           color  ==  black | red | green\n"
         "                      | yellow | blue | magenta\n"
         "                      | cyan | white\n"
         "        -v            Show ttytimer version\n"
         "        -h            Show this page\n",
         argv0);
  exit(exit_code);
}

void init(void) {
  struct sigaction sig;
  ttyclock->bg = COLOR_BLACK;

  initscr();

  cbreak();
  noecho();
  keypad(stdscr, True);
  start_color();
  curs_set(False);
  clear();

  /* Init default terminal color */
  if (use_default_colors() == OK)
    ttyclock->bg = -1;

  /* Init color pair */
  init_pair(0, ttyclock->bg, ttyclock->bg);
  init_pair(1, ttyclock->bg, ttyclock->option.color);
  init_pair(2, ttyclock->option.color, ttyclock->bg);
  refresh();

  /* Init signal handler */
  sig.sa_handler = signal_handler;
  sig.sa_flags = 0;
  sigaction(SIGWINCH, &sig, NULL);
  sigaction(SIGTERM, &sig, NULL);
  sigaction(SIGINT, &sig, NULL);
  sigaction(SIGSEGV, &sig, NULL);

  /* Init global struct */
  ttyclock->running = True;
  ttyclock->stopped = False;
  if (!ttyclock->geo.x)
    ttyclock->geo.x = 0;
  if (!ttyclock->geo.y)
    ttyclock->geo.y = 0;
  if (!ttyclock->geo.a)
    ttyclock->geo.a = 1;
  if (!ttyclock->geo.b)
    ttyclock->geo.b = 1;
  ttyclock->geo.w = SECFRAMEW;
  ttyclock->geo.h = 7;
  ttyclock->tm = localtime(&(ttyclock->lt));
  ttyclock->lt = time(NULL);

  /* Create clock win */
  ttyclock->framewin = newwin(ttyclock->geo.h, ttyclock->geo.w, ttyclock->geo.x,
                              ttyclock->geo.y);
  if (ttyclock->option.box)
    box(ttyclock->framewin, 0, 0);

  /* Create the date win */
  ttyclock->datewin = newwin(DATEWINH, strlen(ttyclock->date.timestr) + 2,
                             ttyclock->geo.x + ttyclock->geo.h - 1,
                             ttyclock->geo.y + (ttyclock->geo.w / 2) -
                                 (strlen(ttyclock->date.timestr) / 2) - 1);

  if (ttyclock->option.box)
    box(ttyclock->datewin, 0, 0);

  clearok(ttyclock->datewin, True);

  set_center();

  nodelay(stdscr, True);

  wrefresh(ttyclock->datewin);

  wrefresh(ttyclock->framewin);
}

void signal_handler(int signal) {
  switch (signal) {
  case SIGWINCH:
    endwin();
    init();
    break;
    /* Interruption signal */
  case SIGINT:
  case SIGTERM:
    ttyclock->running = False;
    /* Segmentation fault signal */
    break;
  case SIGSEGV:
    endwin();
    fprintf(stderr, "Segmentation fault.\n");
    exit(EXIT_FAILURE);
    break;
  }
}

void cleanup(void) {
  if (ttyclock->ttyscr)
    delscreen(ttyclock->ttyscr);
  if (ttyclock)
    free(ttyclock);
}

/* Decrements ttyclock's time by 1 second. */
void update_hour(void) {
  unsigned int seconds =
      ttyclock->date.second[0] * 10 + ttyclock->date.second[1];
  unsigned int minutes =
      ttyclock->date.minute[0] * 10 + ttyclock->date.minute[1];
  unsigned int hours = ttyclock->date.hour[0] * 10 + ttyclock->date.hour[1];

  if (minutes == 0 && seconds == 0)
    hours = hours == 0 ? 59 : hours - 1;
  if (seconds == 0)
    minutes = minutes == 0 ? 59 : minutes - 1;
  seconds = seconds == 0 ? 59 : seconds - 1;

  /* Put it all back into ttyclock. */
  ttyclock->date.hour[0] = hours / 10;
  ttyclock->date.hour[1] = hours % 10;

  ttyclock->date.minute[0] = minutes / 10;
  ttyclock->date.minute[1] = minutes % 10;

  ttyclock->date.second[0] = seconds / 10;
  ttyclock->date.second[1] = seconds % 10;
}

/* Increments ttyclock's time by 1 second. */
void update_hour_stopwatch_mode(void) {
  unsigned int seconds =
      ttyclock->date.second[0] * 10 + ttyclock->date.second[1];
  unsigned int minutes =
      ttyclock->date.minute[0] * 10 + ttyclock->date.minute[1];
  unsigned int hours = ttyclock->date.hour[0] * 10 + ttyclock->date.hour[1];

  if (minutes == 59 && seconds == 59) {
    hours = hours + 1;
    minutes = -1;
  }
  if (seconds == 59) {
    minutes = minutes + 1;
    seconds = -1;
  }
  seconds = seconds + 1;

  /* Put it all back into ttyclock. */
  ttyclock->date.hour[0] = hours / 10;
  ttyclock->date.hour[1] = hours % 10;

  ttyclock->date.minute[0] = minutes / 10;
  ttyclock->date.minute[1] = minutes % 10;

  ttyclock->date.second[0] = seconds / 10;
  ttyclock->date.second[1] = seconds % 10;
}

void draw_number(int n, int x, int y, unsigned int color) {
  int i, sy = y;

  for (i = 0; i < 30; ++i, ++sy) {
    if (sy == y + 6) {
      sy = y;
      ++x;
    }

    wattroff(ttyclock->framewin, A_BLINK);

    wbkgdset(ttyclock->framewin, COLOR_PAIR(number[n][i / 2] * color));
    mvwaddch(ttyclock->framewin, x, sy, ' ');
  }

  wrefresh(ttyclock->framewin);
}

void draw_clock(void) {
  chtype dotcolor = COLOR_PAIR(1);
  unsigned int numcolor = 1;

  /* Change the colours to blink at certain times. */
  if (!ttyclock->option.stopwatch && time(NULL) % 2 == 0) {
    /* dotcolor = COLOR_PAIR(2); */ //blinking of dots each second disabled
    if (time_is_zero())
      numcolor = 2;
  }

  /* Draw hour numbers */
  draw_number(ttyclock->date.hour[0], 1, 1, numcolor);
  draw_number(ttyclock->date.hour[1], 1, 8, numcolor);

  /* 2 dot for number separation */
  wbkgdset(ttyclock->framewin, dotcolor);
  mvwaddstr(ttyclock->framewin, 2, 16, "  ");
  mvwaddstr(ttyclock->framewin, 4, 16, "  ");

  /* Draw minute numbers */
  draw_number(ttyclock->date.minute[0], 1, 20, numcolor);
  draw_number(ttyclock->date.minute[1], 1, 27, numcolor);

  /* Draw the date */

  wattroff(ttyclock->datewin, A_BOLD);

  wbkgdset(ttyclock->datewin, (COLOR_PAIR(2)));
  mvwprintw(ttyclock->datewin, (DATEWINH / 2), 1, ttyclock->date.timestr);
  wrefresh(ttyclock->datewin);

  /* Again 2 dot for number separation */
  wbkgdset(ttyclock->framewin, dotcolor);
  mvwaddstr(ttyclock->framewin, 2, NORMFRAMEW, "  ");
  mvwaddstr(ttyclock->framewin, 4, NORMFRAMEW, "  ");

  /* Draw second numbers */
  draw_number(ttyclock->date.second[0], 1, 39, numcolor);
  draw_number(ttyclock->date.second[1], 1, 46, numcolor);
}

void clock_move(int x, int y, int w, int h) {
  /* Erase border for a clean move */
  wbkgdset(ttyclock->framewin, COLOR_PAIR(0));
  wborder(ttyclock->framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  werase(ttyclock->framewin);
  wrefresh(ttyclock->framewin);

  wbkgdset(ttyclock->datewin, COLOR_PAIR(0));
  wborder(ttyclock->datewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  werase(ttyclock->datewin);
  wrefresh(ttyclock->datewin);

  /* Frame win move */
  mvwin(ttyclock->framewin, (ttyclock->geo.x = x), (ttyclock->geo.y = y));
  wresize(ttyclock->framewin, (ttyclock->geo.h = h), (ttyclock->geo.w = w));

  /* Date win move */
  mvwin(ttyclock->datewin, ttyclock->geo.x + ttyclock->geo.h - 1,
        ttyclock->geo.y + (ttyclock->geo.w / 2) -
            (strlen(ttyclock->date.timestr) / 2) - 1);
  wresize(ttyclock->datewin, DATEWINH, strlen(ttyclock->date.timestr) + 2);

  if (ttyclock->option.box)
    box(ttyclock->datewin, 0, 0);

  if (ttyclock->option.box)
    box(ttyclock->framewin, 0, 0);

  wrefresh(ttyclock->framewin);
  wrefresh(ttyclock->datewin);
}

void set_second(void) {
  int new_w = SECFRAMEW;
  int y_adj;

  for (y_adj = 0; (ttyclock->geo.y - y_adj) > (COLS - new_w - 1); ++y_adj)
    ;

  clock_move(ttyclock->geo.x, (ttyclock->geo.y - y_adj), new_w,
             ttyclock->geo.h);

  set_center();
}

void set_center(void) {
  clock_move((LINES / 2 - (ttyclock->geo.h / 2)),
             (COLS / 2 - (ttyclock->geo.w / 2)), ttyclock->geo.w,
             ttyclock->geo.h);
}

void set_box(Bool b) {
  ttyclock->option.box = b;

  wbkgdset(ttyclock->framewin, COLOR_PAIR(0));
  wbkgdset(ttyclock->datewin, COLOR_PAIR(0));

  if (ttyclock->option.box) {
    wbkgdset(ttyclock->framewin, COLOR_PAIR(0));
    wbkgdset(ttyclock->datewin, COLOR_PAIR(0));
    box(ttyclock->framewin, 0, 0);
    box(ttyclock->datewin, 0, 0);
  } else {
    wborder(ttyclock->framewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wborder(ttyclock->datewin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  }

  wrefresh(ttyclock->datewin);
  wrefresh(ttyclock->framewin);
}

/* Fills two elements from digits into time, handling the -1 case. */
static void fill_ttyclock_time(int *digits, unsigned int *time) {
  if (digits[1] == -1) {
    time[0] = 0;
    if (digits[0] == -1)
      time[1] = 0;
    else
      time[1] = digits[0];
  } else {
    time[0] = digits[0];
    time[1] = digits[1];
  }
}

void key_event(void) {
  int i, c;

  // s delay and ns delay.
  struct timespec length = {1, 0};
  switch (c = wgetch(stdscr)) {
  case 'q':
  case 'Q':
    ttyclock->running = False;
    break;

  case 'r':
  case 'R':
    fill_ttyclock_time(ttyclock->initial_digits, ttyclock->date.hour);
    fill_ttyclock_time(ttyclock->initial_digits + 2, ttyclock->date.minute);
    fill_ttyclock_time(ttyclock->initial_digits + 4, ttyclock->date.second);
    break;

    // toggle state
  case ' ':
    if (ttyclock->stopped) {
      ttyclock->stopped = False;
      set_datewin();
    } else {
      ttyclock->stopped = True;
      strcpy(ttyclock->date.timestr, "Stopped ");
    }

  default:
#ifdef TOOT
    if (time_is_zero() && time(NULL) % 2 == 0)
      toot(500, 1000);
    else
#endif
      nanosleep(&length, NULL);

    for (i = 0; i < 8; ++i) {
      if (c == (i + '0')) {
        ttyclock->option.color = i;
        init_pair(1, ttyclock->bg, i);
        init_pair(2, i, ttyclock->bg);
      }
    }

    break;
  }
}

/* Parses time into ttyclock->date.hour/minute/second. Exits with
 * an error message on bad time format. Sets timestr to what was
 * parsed.
 * time format: hh:mm:ss, where all but the colons are optional.
 */
static void parse_time_arg(char *time) {
  int digits[N_TIME_DIGITS];
  for (int i = 0; i < N_TIME_DIGITS; ++i)
    digits[i] = -1;

  int i = 0, remaining = 2;
  while (*time != '\0') {
    if (isdigit(*time)) {
      if (remaining == 0) {
        puts("Too many digits in time argument");
        exit(EXIT_FAILURE);
      }

      digits[i] = *time - '0';
      ++i;
      --remaining;
    } else if (*time == ':') {
      i += remaining;
      remaining = 2;
    } else {
      puts("Invalid character in time argument");
      exit(EXIT_FAILURE);
    }

    ++time;
  }

  fill_ttyclock_time(digits, ttyclock->date.hour);
  fill_ttyclock_time(digits + 2, ttyclock->date.minute);
  fill_ttyclock_time(digits + 4, ttyclock->date.second);
  memcpy(ttyclock->initial_digits, digits, N_TIME_DIGITS * sizeof(int));

  if (ttyclock->option.datewin) {
    ttyclock->date.timestr[0] = ttyclock->date.hour[0] + '0';
    ttyclock->date.timestr[1] = ttyclock->date.hour[1] + '0';
    ttyclock->date.timestr[2] = ':';
    ttyclock->date.timestr[3] = ttyclock->date.minute[0] + '0';
    ttyclock->date.timestr[4] = ttyclock->date.minute[1] + '0';
    ttyclock->date.timestr[5] = ':';
    ttyclock->date.timestr[6] = ttyclock->date.second[0] + '0';
    ttyclock->date.timestr[7] = ttyclock->date.second[1] + '0';
    ttyclock->date.timestr[8] = '\0';
  } else {
    strcpy(ttyclock->date.timestr, "        ");
  }

  strcpy(ttyclock->inital_timestr, ttyclock->date.timestr);

  set_datewin();
}

void time_str_to_arr(char *timeStr, int digits[]) {
		for (int i = 0; i < N_TIME_DIGITS; ++i)
			digits[i] = 0;

		int i = 0, remaining = 2;
		while (*timeStr != '\0') {
			if (isdigit(*timeStr)) {
				if (remaining == 0) {
					puts("Too many digits in time argument");
					exit(EXIT_FAILURE);
				}

				digits[i] = *timeStr - '0';
				++i;
				--remaining;
			} else if (*timeStr == ':') {
				i += remaining;
				remaining = 2;
			} else {
				puts("Invalid character in time argument");
				exit(EXIT_FAILURE);
			}

			++timeStr;
		}
}


void difference_between_time_period(int start[],
                                 int stop[],
                                 int diff[]) {
	// seconds first digit
	while(stop[5] > start[5]) {
		--start[4];
		start[5] += 10;
	}
	diff[5] = start[5] - stop[5];
	// seconds second digit
	while(stop[4] > start[4]) {
		--start[3];
		start[4] += 6;
	}
	diff[4] = start[4] - stop[4];
	// minutes first digit
	while(stop[3] > start[3]) {
		--start[2];
		start[3] += 10;
	}
	diff[3] = start[3] - stop[3];
	// minutes second digit
	while(stop[2] > start[2]) {
		--start[1];
		start[2] += 6;
	}
	diff[2] = start[2] - stop[2];
	// hours first digit
	while(stop[1] > start[1]) {
		--start[0];
		start[1] += 10;
	}
	diff[1] = start[1] - stop[1];
	diff[0] = start[0] - stop[0];

}



void time_difference(char *timeStr) {
	int digits[N_TIME_DIGITS];
	time_str_to_arr(timeStr, digits);

	time_t t = time(NULL);
  struct tm tm = *localtime(&t);

	int digitsNow[N_TIME_DIGITS];

	digitsNow[0] = tm.tm_hour / 10;
	digitsNow[1] = tm.tm_hour % 10;
	digitsNow[2] = tm.tm_min / 10;
	digitsNow[3] = tm.tm_min % 10;
	digitsNow[4] = tm.tm_sec / 10;
	digitsNow[5] = tm.tm_sec % 10;

	int diff[N_TIME_DIGITS];
	difference_between_time_period(digits, digitsNow, diff);

	int negative = 0;
	for (int i = 0; i < N_TIME_DIGITS; ++i) {
		if (diff[i] < 0) {
			negative = 1;
			break;
		}
	}

	int hour_24[] = {2,4,0,0,0,0};
	if (negative) {
		difference_between_time_period(digitsNow, digits, diff);
		difference_between_time_period(hour_24, diff, diff);
	}


	timeStr[0] = diff[0] + '0';
	timeStr[1] = diff[1] + '0';
	timeStr[2] = ':';
	timeStr[3] = diff[2] + '0';
	timeStr[4] = diff[3] + '0';
	timeStr[5] = ':';
	timeStr[6] = diff[4] + '0';
	timeStr[7] = diff[5] + '0';
	timeStr[8] = '\0'; 
}





void set_datewin(void) {
  if (ttyclock->option.stopwatch || !ttyclock->option.datewin) {
    strcpy(ttyclock->date.timestr, "        ");
  } else {
    strcpy(ttyclock->date.timestr, ttyclock->inital_timestr);
  }
}

/* Converts the name of a colour to its ncurses number. Case insensitive. */
int color_name_to_number(const char *color) {

  if (strcasecmp(color, "black") == 0)
    return COLOR_BLACK;
  else if (strcasecmp(color, "red") == 0)
    return COLOR_RED;
  else if (strcasecmp(color, "green") == 0)
    return COLOR_GREEN;
  else if (strcasecmp(color, "yellow") == 0)
    return COLOR_YELLOW;
  else if (strcasecmp(color, "blue") == 0)
    return COLOR_BLUE;
  else if (strcasecmp(color, "magenta") == 0)
    return COLOR_MAGENTA;
  else if (strcasecmp(color, "cyan") == 0)
    return COLOR_CYAN;
  else if (strcasecmp(color, "white") == 0)
    return COLOR_WHITE;
  else
    return -1;
}

int main(int argc, char **argv) {
  int c;

  /* Alloc ttyclock */
  ttyclock = malloc(sizeof(ttyclock_t));
  assert(ttyclock != NULL);
  memset(ttyclock, 0, sizeof(ttyclock_t));

  /* Default color */
  ttyclock->option.color = COLOR_GREEN; /* COLOR_GREEN = 2 */

  atexit(cleanup);

  int color;
  while ((c = getopt(argc, argv, "tvdhxC:")) != -1) {
    switch (c) {
    case 'h':
      usage(argv[0], EXIT_SUCCESS);
      break;
    case 'v':
      puts("ttytimer v0.1");
      exit(EXIT_SUCCESS);
      break;
    case 'C':
      if ((color = color_name_to_number(optarg)) != -1) {
        ttyclock->option.color = color;
      } else {
        printf("Invalid color specified: %s\n", optarg);
        exit(EXIT_FAILURE);
      }
    case 'd':
      ttyclock->option.datewin = True;
      break;
    case 'x':
      ttyclock->option.box = True;
      break;
		case 't':
			if (argc != 3) {
				usage(argv[0], EXIT_FAILURE);
			}
			time_difference(argv[optind]);
			break;
    default:
      usage(argv[0], EXIT_FAILURE);
      break;
    }
  }

  /* We're missing the final time argument. */
  if (optind == argc) {
    ttyclock->option.stopwatch = True;
    argv[optind] = "0:0:0";
  }

  parse_time_arg(argv[optind]);

  /* Ensure input is anything but 0. */
  if (!ttyclock->option.stopwatch && time_is_zero()) {
    puts("Time argument is zero");
    exit(EXIT_FAILURE);
  }

  init();
  attron(A_BLINK);

  if (!ttyclock->option.stopwatch)
    while (ttyclock->running) {
      draw_clock();
      key_event();
      if (!ttyclock->stopped && !time_is_zero())
        update_hour();
    }
  else
    while (ttyclock->running) {
      draw_clock();
      key_event();
      if (!ttyclock->stopped)
        update_hour_stopwatch_mode();
    }

  endwin();

  return 0;
}
