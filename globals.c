#include "globals.h"

int g_x = 800;
int g_y = 600;

/* Стойност от 72 е приемлива при използване върху вградената ми i915 карта :) */
size_t no_segments = 72;

size_t buffer_size = 0;
struct vertex **vertex_buffer = NULL;

size_t polygon_size = 0;
struct polygon *polygon_buffer = NULL;
