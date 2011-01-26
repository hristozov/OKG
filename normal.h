#pragma once

#include <stdio.h>
#include "buffer.h"
#include "globals.h"

void calculateNormal(struct point*, struct point*, struct point*, struct point*);
void vertexNormal(size_t, size_t);
void calculateVertexNormals();