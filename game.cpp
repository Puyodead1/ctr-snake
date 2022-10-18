#include "game.h"


int gridX, gridY;
float width = 1.0f;
int posX = 20, posY = 20;
short sDirection = RIGHT;

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

	if (posX > COLUMNS)
		posX = 0;
	if (posY > ROWS)
		posY = 0;

	if (posX < 0)
		posX = COLUMNS;
	if (posY < 0)
		posY = ROWS;

	C2D_DrawRectSolid(posX * GRID_UNIT_SIZE, posY * GRID_UNIT_SIZE, 0.0f, 10, 10, COLOR_RED);
}