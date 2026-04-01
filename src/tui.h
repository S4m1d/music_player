#ifndef TUI_H
#define TUI_H
#include <ncurses.h>

void *run_tui(void *arg);

void draw_playlist(WINDOW *pl_win, char *tracks[], int tracks_num,
                   int cur_track_idx);
void draw_cur_track(WINDOW *win, char *cur_track_name);

char **scan_tracks(const char *path, const int page_size, int *tracks_count);

#endif
