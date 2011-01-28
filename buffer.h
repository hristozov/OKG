/*
 * Тук се намират функциите за работа с буферите за върхове и полигони
 */

#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"

void buffer_init();
void buffer_resize(size_t);
void buffer_kill();
void add_point(float, float, float);
void fill_polygon_buffer();
#if SORT_CONTROL_POINTS == 1
void swap_indexes(size_t, size_t);
void sort_control_points();
#endif
