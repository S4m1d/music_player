#include <dirent.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int PL_WIDTH = 30;
int PL_HEIGHT = 10;
int CUR_TRACK_WIDTH = 30;
int CUR_TRACK_HEIGHT = 3;

void draw_playlist(WINDOW *pl_win, char *tracks[], int tracks_num,
                   int cur_track_idx);
void draw_cur_track(WINDOW *win, char *cur_track_name);

char **scan_tracks(const char *path, const int page_size, int *tracks_count);

int main() {
  WINDOW *pl_win;
  WINDOW *cur_track_win;
  int cur_hl_track_idx = 0;
  int cur_pl_track_idx = -1;

  int tracks_count;
  char **tracks = scan_tracks("assets", 10, &tracks_count);
  if (!tracks) {
    return -1;
  }

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
  draw_playlist(pl_win, tracks, tracks_count, cur_hl_track_idx);

  bool is_terminated = false;
  while (!is_terminated) {
    int ch = getch();
    switch (ch) {
    case 'j':
      if (cur_hl_track_idx + 1 < tracks_count) {
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
    draw_playlist(pl_win, tracks, tracks_count, cur_hl_track_idx);
  }

  endwin();

  for (int i = 0; i < tracks_count; i++) {
    free(tracks[i]);
  }
  free(tracks);

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

char *MP3_EXT = ".mp3";

char **scan_tracks(const char *path, const int page_size, int *tracks_count) {
  DIR *dir = opendir(path);
  if (!dir) {
    perror("opendir");
    *tracks_count = 0;
    return NULL;
  }

  int n = 0;
  struct dirent *entry;
  while (n < page_size && (entry = readdir(dir)) != NULL) {
    char *dot = strchr(entry->d_name, '.');
    if (dot && strcmp(dot, MP3_EXT) == 0) {
      n++;
    }
  }

  char **track_names = malloc(n * sizeof(char *));
  if (!track_names) {
    closedir(dir);
    perror("malloc track_names");
    *tracks_count = 0;
    return NULL;
  }

  rewinddir(dir);
  int i = 0;
  while (n < page_size && (entry = readdir(dir)) != NULL) {
    char *dot = strchr(entry->d_name, '.');
    if (dot && strcmp(dot, MP3_EXT) == 0) {
      int len = dot - entry->d_name;
      track_names[i] = malloc(len + 1); // +1 for null terminator
      strncpy(track_names[i], entry->d_name, dot - entry->d_name);
      i++;
    }
  }

  *tracks_count = i;
  return track_names;
}
