#pragma once
#ifndef COMMON_H
#define COMMON_H

#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

#define GRID_UNIT_SIZE 10
#define ROWS SCREEN_HEIGHT / GRID_UNIT_SIZE // x
#define COLUMNS SCREEN_WIDTH / GRID_UNIT_SIZE // y
#define FPS 100000

#define RIGHT 1
#define LEFT 2
#define UP 3
#define DOWN 4
#define NONE = 0

#endif