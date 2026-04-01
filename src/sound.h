#ifndef SOUND_H
#define SOUND_H

#include <ao/ao.h>
#include <mpg123.h>
#include <stdatomic.h>

int init_sound();
void deinit_sound();
void *play_track(void *arg);

typedef struct {
  atomic_int stop;
  char filepath[256];
  int driver_id;
} PlayerArgs;

#endif
