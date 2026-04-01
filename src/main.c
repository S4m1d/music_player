#include "tui.h"
#include <pthread.h>

int main() {
  pthread_t t;

  pthread_create(&t, NULL, run_tui, NULL);
  pthread_join(t, NULL);

  return 0;
}
