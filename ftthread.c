
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <pthread.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define DIE(msg) do { fprintf (stderr, msg "\n"); abort (); } while (0)

#define MAX_NUM_THREADS 4096

int num_iters = 100;
int ppem = 100;
const char *font_file = NULL;

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

    FT_Load_Glyph (face, i % face->num_glyphs, 0);

    if ((i & 63) == 0)
    {
      pthread_mutex_lock (&lock);
      FT_Done_Face (face);
      if (FT_New_Face (ft_library, font_file, 0, &face))
	DIE ("Failed recreating face.");
      pthread_mutex_unlock (&lock);
    }
  }

  if (face)
    FT_Done_Face (face);

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
	     "usage: ftthread fontfile.ttf [numthreads] [numiters] [ppem]\n"
	     "\n"
	     "numthreads, numiters, and ppem default to 100.\n");
    exit (1);
  }
  font_file = argv[1];
  if (argc > 2)
    num_threads = atoi (argv[2]);
  if (argc > 3)
    num_iters = atoi (argv[3]);
  if (argc > 4)
    ppem = atoi (argv[4]);

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
