#include "sound.h"
#include <stdio.h>
#include <string.h>

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
