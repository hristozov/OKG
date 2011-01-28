/*
 * Тук са функциите за изчисляване на нормални вектори
 */

#pragma once

#include <stdio.h>
#include "buffer.h"
#include "globals.h"

void calculate_normal(struct point*, struct point*, struct point*, struct point*);

#if SMOOTH_SHADING == 1
void vertex_normal(size_t, size_t);
void calculate_vertex_normals();
#endif
