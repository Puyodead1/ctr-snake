#include "game.h"


int gridX, gridY;
float width = 1.0f;
int posX = 20, posY = 20;
short sDirection = RIGHT;
extern bool gameOver;
float lerpT = 0.7f;
float step = 0.05f;
C2D_Text text[4];
C2D_TextBuf textBuf;

void gameInit()
{
	textBuf = C2D_TextBufNew(4096);

	C2D_TextParse(&text[0], textBuf, "Game Over");
	C2D_TextParse(&text[1], textBuf, "Your Score: 20");
	C2D_TextParse(&text[2], textBuf, "High Score: 40");

	C2D_TextOptimize(&text[0]);
	C2D_TextOptimize(&text[1]);
	C2D_TextOptimize(&text[2]);
}

void gameRender()
{
	if (lerpT > 1.0f)
		step = -0.05f;
	else if (lerpT < 0.7f)
		step = +0.05f;

	lerpT += step;
}

void gameExit()
{
	// delete the buffers
	C2D_TextBufDelete(textBuf);
}

void gameReset()
{
	lerpT = 0.7f;
	step = 0.05f;

	// reset snake position
	posX = 20;
	posY = 20;

	gameOver = false;
	sDirection = RIGHT;
}

void initGrid(int x, int y)
{
	printf("Columns: %i\n", x);
	printf("Rows: %i\n", y);
	gridX = x;
	gridY = y;
}

void drawGrid()
{
	for (int x = 0; x < gridX; x++)
	{
		for (int y = 0; y < gridY; y++)
		{
			drawGridUnit(x * GRID_UNIT_SIZE, y * GRID_UNIT_SIZE);
		}
	}
}

void drawGridUnit(int x, int y)
{
	u32 COLOR_RED = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
	C2D_DrawLine(x, y, COLOR_RED, x + GRID_UNIT_SIZE, y, COLOR_RED, width, 0.0f);
	C2D_DrawLine(x + GRID_UNIT_SIZE, y, COLOR_RED, x + GRID_UNIT_SIZE, y + GRID_UNIT_SIZE, COLOR_RED, width, 0.0f);
	C2D_DrawLine(x + GRID_UNIT_SIZE, y + GRID_UNIT_SIZE, COLOR_RED, x, y + GRID_UNIT_SIZE, COLOR_RED, width, 0.0f);
	C2D_DrawLine(x, y + GRID_UNIT_SIZE, COLOR_RED, x, y, COLOR_RED, width, 0.0f);
}

void drawSnake()
{
	u32 COLOR_RED = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);

	if (sDirection == RIGHT)
		posX++;
	else if (sDirection == LEFT)
		posX--;
	else if (sDirection == UP)
		posY--;
	else if (sDirection == DOWN)
		posY++;

	C2D_DrawRectSolid(posX * GRID_UNIT_SIZE, posY * GRID_UNIT_SIZE, 0.0f, 10, 10, COLOR_RED);

	if (posX == -1 || posX == gridX || posY == -1 || posY == gridY)
		gameOver = true;
}

void drawGameOver()
{
	u32 COLOR = C2D_Color32(0xFF, 0xFF, 0xFF, 0xF0);
	C2D_DrawRectSolid(60.0f, 60.0f, 0.0f, SCREEN_WIDTH - (60 * 2), SCREEN_HEIGHT - (60 * 2), COLOR);

	C2D_TextParse(&text[3], textBuf, "Press A to restart");
	C2D_TextOptimize(&text[3]);

	C2D_DrawText(&text[0], C2D_AlignCenter, 200.0f, 70.0f, 0.5f, 1.0f, 1.0f, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
	C2D_DrawText(&text[1], C2D_AlignCenter, 200.0f, 100.0f, 0.5f, 0.8f, 0.8f, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
	C2D_DrawText(&text[2], C2D_AlignCenter, 200.0f, 120.0f, 0.5f, 0.8f, 0.8f, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
	C2D_DrawText(&text[3], C2D_AlignCenter, 200.0f, 150.0f, 0.5f, lerpT, lerpT, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
}