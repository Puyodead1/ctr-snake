#pragma once
#ifndef GAME_H
#define GAME_H

#include "common.h"
#include <time.h>

void gameInit();
void gameRender();
void gameExit();
void gameReset();
void initGrid(int, int);
void drawGrid();
void drawGridUnit(int, int);
void drawSnake();
void drawGameOver();
void drawFood();
void randomFoodPos(int &x, int &y);
void checkBounds();

#endif