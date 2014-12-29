
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define DIE(msg) do { fprintf (stderr, msg "\n"); abort (); } while (0)

int num_iters = 100;
int ppem = 100;
FT_Library ft_library;
const char *font_file = NULL;

static void *
draw_thread (void *arg)
{
  int i;
  FT_Face ft_face = (FT_Face) arg;

  for (i = 0; i < num_iters; i++)
  {
    FT_Load_Glyph (ft_face, 50, 0);
//    printf("%d\n", i);
  }

  return NULL;
}

int
main (int argc, char **argv)
{
  int num_threads = 100;

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

  pthread_t threads[num_threads];
  FT_Face thread_face[num_threads];
  int i;

  FT_Init_FreeType (&ft_library);

  for (i = 0; i < num_threads; i++)
  {
    if (FT_New_Face (ft_library, font_file, 0, &thread_face[i]))
      DIE ("Failed opening face.");
    if (FT_Set_Char_Size (thread_face[i], ppem, ppem, 0, 0))
      DIE ("FT_Set_Char_Size failed.");
    if (pthread_create (&threads[i], NULL, draw_thread, thread_face[i]) != 0)
      DIE ("pthread_create() failed.");
  }

  for (i = 0; i < num_threads; i++)
  {
    if (pthread_join (threads[i], NULL) != 0)
      DIE ("pthread_join() failed.");
    FT_Done_Face ((FT_Face) thread_face[i]);
  }

  FT_Done_FreeType (ft_library);

  return 0;
}
