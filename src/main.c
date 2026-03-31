#include <ao/ao.h>
#include <dirent.h>
#include <mpg123.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *run_tui(void *arg);
int init_sound();
void deinit_sound();

int main() {
  pthread_t t;

  pthread_create(&t, NULL, run_tui, NULL);
  pthread_join(t, NULL);

  return 0;
}

int PL_WIDTH = 30;
int PL_HEIGHT = 10;
int CUR_TRACK_WIDTH = 30;
int CUR_TRACK_HEIGHT = 3;

// player definitions
void *play_track(void *arg);

typedef struct {
  atomic_int stop;
  char filepath[256];
  int driver_id;
} PlayerArgs;
//

void draw_playlist(WINDOW *pl_win, char *tracks[], int tracks_num,
                   int cur_track_idx);
void draw_cur_track(WINDOW *win, char *cur_track_name);

char **scan_tracks(const char *path, const int page_size, int *tracks_count);

void *run_tui(void *arg) {
  (void)arg;
  WINDOW *pl_win;
  WINDOW *cur_track_win;
  int cur_hl_track_idx = 0;
  int cur_pl_track_idx = -1;

  int driver_id = init_sound();

  int tracks_count;
  char **tracks = scan_tracks("assets", 10, &tracks_count);
  if (!tracks) {
    printf("failed to scan for tracks");
    // TODO: return some other value on error
    return NULL;
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

  PlayerArgs p_args;
  memset(&p_args, 0, sizeof(p_args));
  p_args.stop = 0;
  p_args.driver_id = driver_id;

  pthread_t player_thread;

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

      p_args.stop = 1;
      if (player_thread) {
        pthread_join(player_thread, NULL);
      }

      char filepath[256];
      snprintf(filepath, sizeof(filepath), "assets/%s.mp3",
               tracks[cur_pl_track_idx]);
      strncpy(p_args.filepath, filepath, sizeof(p_args.filepath));
      p_args.filepath[strlen(filepath)] = '\0';

      p_args.stop = 0;
      pthread_create(&player_thread, NULL, play_track, &p_args);

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

  p_args.stop = 1;
  if (player_thread) {
    pthread_join(player_thread, NULL);
  }

  endwin();

  deinit_sound();

  for (int i = 0; i < tracks_count; i++) {
    free(tracks[i]);
  }
  free(tracks);

  return NULL;
}

int init_sound() {
  mpg123_init();
  ao_initialize();

  return ao_driver_id("pulse");
}

void deinit_sound() {
  mpg123_exit();
  ao_shutdown();
}

void *play_track(void *arg) {
  PlayerArgs *p_args = (PlayerArgs *)arg;
  mpg123_handle *mh = mpg123_new(NULL, NULL);
  mpg123_open(mh, (char *)p_args->filepath);

  long rate;
  int channels, enc;
  int rc = mpg123_getformat(mh, &rate, &channels, &enc);
  if (rc != MPG123_OK) {
    // TODO: handle error
    return NULL;
  }

  ao_sample_format fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.bits = mpg123_encsize(enc) * 8;
  fmt.rate = rate;
  fmt.channels = channels;
  fmt.byte_format = AO_FMT_NATIVE;
  fmt.matrix = 0;

  ao_device *ao_dev = ao_open_live(p_args->driver_id, &fmt, NULL);
  if (!ao_dev) {
    // TODO: handle error
    return NULL;
  }

  unsigned char buf[4096];
  size_t done;
  while (!p_args->stop &&
         mpg123_read(mh, buf, sizeof(buf), &done) == MPG123_OK) {
    ao_play(ao_dev, (char *)buf, done);
  }

  // TODO: cleanup
  ao_close(ao_dev);
  mpg123_close(mh);
  mpg123_delete(mh);

  return NULL;
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
      track_names[i][len] = '\0';
      i++;
    }
  }

  *tracks_count = i;
  return track_names;
}
