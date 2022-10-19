#pragma once
#ifndef GAME_H
#define GAME_H

#include "common.h"

void gameInit();
void gameRender();
void gameExit();
void gameReset();
void initGrid(int, int);
void drawGrid();
void drawGridUnit(int, int);
void drawSnake();
void drawGameOver();

#endif