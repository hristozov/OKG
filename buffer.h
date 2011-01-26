#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"

//extern size_t no_segments;

void buffer_init();
void buffer_resize(size_t);
void buffer_kill();
void add_point(float, float, float);
