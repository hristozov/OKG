/*
 * Тук се намират функциите за работа с буферите за върхове и полигони
 */

#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"

void buffer_init();
void buffer_resize(size_t);
void buffer_kill();
void add_point(float, float, float);
void fill_polygon_buffer();
