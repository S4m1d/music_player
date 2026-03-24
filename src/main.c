#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>

int PL_WIDTH = 30;
int PL_HEIGHT = 10;
int CUR_TRACK_WIDTH = 30;
int CUR_TRACK_HEIGHT = 3;

void draw_playlist(WINDOW *pl_win, char *tracks[], int tracks_num,
                   int cur_track_idx);
void draw_cur_track(WINDOW *win, char *cur_track_name);

int main() {
  WINDOW *pl_win;
  WINDOW *cur_track_win;
  int cur_hl_track_idx = 0;
  int cur_pl_track_idx = -1;

  char *tracks[] = {
      "Tepsen.mp3",
      "Chistiy.mp3",
      "NeverFadeAway.mp3",
  };
  int tracks_num = sizeof(tracks) / sizeof(char *);

  initscr();
  cbreak();
  noecho();
  refresh();
  curs_set(0);

  int rows, cols;
  getmaxyx(stdscr, rows, cols);

  int start_y = (rows - (CUR_TRACK_HEIGHT + PL_HEIGHT)) / 2;

  int cur_track_start_x = (cols - CUR_TRACK_WIDTH) / 2;
  int cur_track_start_y = start_y;

  int pl_start_x = (cols - PL_WIDTH) / 2;
  int pl_start_y = start_y + CUR_TRACK_HEIGHT;

  cur_track_win = newwin(CUR_TRACK_HEIGHT, CUR_TRACK_WIDTH, cur_track_start_y,
                         cur_track_start_x);
  pl_win = newwin(PL_HEIGHT, PL_WIDTH, pl_start_y, pl_start_x);

  draw_cur_track(cur_track_win, NULL);
  draw_playlist(pl_win, tracks, tracks_num, cur_hl_track_idx);

  bool is_terminated = false;
  while (!is_terminated) {
    int ch = getch();
    switch (ch) {
    case 'j':
      if (cur_hl_track_idx + 1 < tracks_num) {
        cur_hl_track_idx++;
      }

      break;
    case 'k':
      if (cur_hl_track_idx - 1 >= 0) {
        cur_hl_track_idx--;
      }

      break;
    case 'p':
      cur_pl_track_idx = cur_hl_track_idx;

      break;
    case 'q':
      is_terminated = true;

      break;
    }

    if (cur_pl_track_idx > -1) {
      draw_cur_track(cur_track_win, tracks[cur_pl_track_idx]);
    }
    draw_playlist(pl_win, tracks, tracks_num, cur_hl_track_idx);
  }

  endwin();

  return 0;
}

void draw_playlist(WINDOW *win, char *tracks[], int tracks_num,
                   int cur_track_idx) {

  int x, y;
  y = 1;
  x = 2;

  wclear(win);
  box(win, 0, 0);

  for (int i = 0; i < tracks_num; i++) {
    if (i == cur_track_idx) {
      wattron(win, A_REVERSE);
      mvwprintw(win, y, x, "> %d. %s", i + 1, tracks[i]);
      wattroff(win, A_REVERSE);
    } else {
      mvwprintw(win, y, x, "%d. %s", i + 1, tracks[i]);
    }

    y++;
  }

  wrefresh(win);
}

void draw_cur_track(WINDOW *win, char *cur_track_name) {
  wclear(win);

  box(win, 0, 0);
  if (cur_track_name != NULL) {
    mvwprintw(win, 1, 1, "%s", cur_track_name);
  }

  wrefresh(win);
}
