
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <pthread.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define DIE(msg) do { fprintf (stderr, msg "\n"); abort (); } while (0)

#define MAX_NUM_THREADS 4096

const char *font_file = NULL;
int num_iters = 100;
int ppem = 100;
int load_flags = 0;

pthread_mutex_t lock;
FT_Library ft_library;

static void *
draw_thread (void *arg)
{
  int i;
  FT_Face face = NULL;
  arg = arg;

  for (i = 0; i < num_iters; i++)
  {
    if (!face)
    {
      pthread_mutex_lock (&lock);
      if (FT_New_Face (ft_library, font_file, 0, &face))
	DIE ("Failed creating face.");
      pthread_mutex_unlock (&lock);
      if (FT_Set_Char_Size (face, ppem, ppem, 0, 0))
	DIE ("FT_Set_Char_Size failed.");
    }

    FT_Load_Glyph (face, i % face->num_glyphs, load_flags);

    if (i % 1000 == 0)
    {
      pthread_mutex_lock (&lock);
      FT_Done_Face (face);
      if (FT_New_Face (ft_library, font_file, 0, &face))
	DIE ("Failed recreating face.");
      pthread_mutex_unlock (&lock);
    }
  }

  if (face)
  {
    pthread_mutex_lock (&lock);
    FT_Done_Face (face);
    pthread_mutex_unlock (&lock);
  }

  return NULL;
}

int
main (int argc, char **argv)
{
  int num_threads = 100;
  pthread_t threads[MAX_NUM_THREADS];
  int i;

  if (argc < 2)
  {
    fprintf (stderr,
	     "usage: ftthread fontfile.ttf [numthreads] [numiters] [ppem] [loadflags]\n"
	     "\n"
	     "numthreads, numiters, and ppem default to 100.\n"
	     "loadflags defaults to 0.  Useful flags to logically or:\n"
	     "NO_HINTING=2\nRENDER=4\nFORCE_AUTOHINT=32\nMONOCHROME=4096\n"
	     "NO_AUTOHINT=32768\nCOLOR=1048576\n"
	     );
    exit (1);
  }
  font_file = argv[1];
  if (argc > 2)
    num_threads = atoi (argv[2]);
  if (argc > 3)
    num_iters = atoi (argv[3]);
  if (argc > 4)
    ppem = atoi (argv[4]);
  if (argc > 5)
    load_flags = atoi (argv[5]);

  assert (num_threads <= MAX_NUM_THREADS);

  pthread_mutex_init (&lock, NULL);
  FT_Init_FreeType (&ft_library);

  for (i = 0; i < num_threads; i++)
  {
    if (pthread_create (&threads[i], NULL, draw_thread, NULL) != 0)
      DIE ("pthread_create() failed.");
  }

  for (i = 0; i < num_threads; i++)
  {
    if (pthread_join (threads[i], NULL) != 0)
      DIE ("pthread_join() failed.");
  }

  FT_Done_FreeType (ft_library);
  pthread_mutex_destroy (&lock);

  return 0;
}
