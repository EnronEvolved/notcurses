#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <notcurses/notcurses.h>

static bool fddone;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int
cb(struct ncfdplane* ncfd, const void* data, size_t len, void* curry){
  int ret = -1;
  if(ncplane_putstr(ncfdplane_plane(ncfd), data) >= 0){
    if(!notcurses_render(ncplane_notcurses(ncfdplane_plane(ncfd)))){
      ret = 0;
    }
  }
  (void)len;
  (void)curry;
  return ret;
}

static int
eofcb(struct ncfdplane* ncfd, int nerrno, void* curry){
  (void)ncfd;
  (void)nerrno;
  (void)curry;
  pthread_mutex_lock(&lock);
  fddone = true;
  pthread_mutex_unlock(&lock);
  pthread_cond_signal(&cond);
  return nerrno;
}

int main(int argc, char** argv){
  if(argc < 2){
    fprintf(stderr, "usage: procroller binary [ args... ]\n");
    return EXIT_FAILURE;
  }
  setlocale(LC_ALL, "");
  notcurses_options opts = {
    .inhibit_alternate_screen = true,
    .flags = NCOPTION_INHIBIT_SETLOCALE,
  };
  struct notcurses* nc = notcurses_init(&opts, NULL);
  if(nc == NULL){
    return EXIT_FAILURE;
  }
  struct ncplane* n = notcurses_stdplane(nc);
  ncsubproc_options nopts = {};
  struct ncsubproc* nsproc = ncsubproc_createvp(n, &nopts, *argv, argv, cb, eofcb);
  pthread_mutex_lock(&lock);
  while(!fddone){
    pthread_cond_wait(&cond, &lock);
  }
  pthread_mutex_unlock(&lock);
  ncsubproc_destroy(nsproc);
  if(notcurses_stop(nc)){
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
