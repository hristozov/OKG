#include "globals.h"

int g_x = 800;
int g_y = 600;

size_t no_segments = 72;

size_t buffer_size = 0;
struct vertex **vertex_buffer = NULL;

size_t polygon_size = 0;
struct polygon *polygon_buffer = NULL;
