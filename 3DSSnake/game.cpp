#include "game.h"

int gridX, gridY;
int posX = 20, posY = 20;
int foodX, foodY;
int score = 0;
int highScore = 0;
short sDirection = RIGHT;
bool gameOver = false;
bool foodEaten = true;
float lerpT = 0.7f; 
float step = 0.05f; // lerp step, used for animating text in game over popup
float width = 1.0f;

C2D_Text text[2];
C2D_TextBuf staticTextBuf, dynTextBuf;

u32 MESSAGE_BOX_BG_COLOR = C2D_Color32(0xFF, 0xFF, 0xF1, 0xCC); // off-white
u32 GRID_COLOR = C2D_Color32(0xFF, 0x00, 0x00, 0xFF); // red
u32 SNAKE_COLOR = C2D_Color32(0x00, 0xFF, 0x00, 0xA5); // green
u32 FOOD_COLOR = C2D_Color32(0xFF, 0x00, 0x00, 0xA5);

void gameInit()
{
	staticTextBuf = C2D_TextBufNew(4096);
	dynTextBuf = C2D_TextBufNew(4096);

	C2D_TextParse(&text[0], staticTextBuf, "Game Over");
	C2D_TextParse(&text[1], staticTextBuf, "Press A to restart");

	C2D_TextOptimize(&text[0]);
	C2D_TextOptimize(&text[1]);
}

void gameRender()
{
	if (lerpT > 1.0f)
		step = -0.05f;
	else if (lerpT < 0.7f)
		step = +0.05f;

	lerpT += step;

	drawGrid();
	drawFood();
	drawSnake();
	checkBounds();
	if (gameOver)
	{
		drawGameOver();
	}
}

void gameExit()
{
	// delete the buffers
	C2D_TextBufDelete(staticTextBuf);
	C2D_TextBufDelete(dynTextBuf);
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

	foodEaten = true;

	score = 0;
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
	C2D_DrawLine(x, y, GRID_COLOR, x + GRID_UNIT_SIZE, y, GRID_COLOR, width, 0.0f);
	C2D_DrawLine(x + GRID_UNIT_SIZE, y, GRID_COLOR, x + GRID_UNIT_SIZE, y + GRID_UNIT_SIZE, GRID_COLOR, width, 0.0f);
	C2D_DrawLine(x + GRID_UNIT_SIZE, y + GRID_UNIT_SIZE, GRID_COLOR, x, y + GRID_UNIT_SIZE, GRID_COLOR, width, 0.0f);
	C2D_DrawLine(x, y + GRID_UNIT_SIZE, GRID_COLOR, x, y, GRID_COLOR, width, 0.0f);
}

void drawSnake()
{
	if (sDirection == RIGHT)
		posX++;
	else if (sDirection == LEFT)
		posX--;
	else if (sDirection == UP)
		posY--;
	else if (sDirection == DOWN)
		posY++;

	C2D_DrawRectSolid(posX * GRID_UNIT_SIZE, posY * GRID_UNIT_SIZE, 0.0f, 10, 10, SNAKE_COLOR);
}

void drawGameOver()
{
	C2D_TextBufClear(dynTextBuf);

	// draw the popup background
	C2D_DrawRectSolid(60.0f, 60.0f, 0.0f, SCREEN_WIDTH - (60 * 2), SCREEN_HEIGHT - (60 * 2), MESSAGE_BOX_BG_COLOR);

	// draw static text (game over, and press a to restart)
	C2D_DrawText(&text[0], C2D_AlignCenter, 200.0f, 70.0f, 0.5f, 1.0f, 1.0f, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
	C2D_DrawText(&text[1], C2D_AlignCenter, 200.0f, 150.0f, 0.5f, lerpT, lerpT, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));

	// create dynamic text
	char scoreTextBuf[160], highScoreTextBuf[160];
	C2D_Text scoreText, highScoreText;

	snprintf(scoreTextBuf, sizeof(scoreTextBuf), "Your Score: %i", score);
	snprintf(highScoreTextBuf, sizeof(highScoreTextBuf), "High Score: %i", highScore);

	// parse the dynamic text
	C2D_TextParse(&scoreText, dynTextBuf, scoreTextBuf);
	C2D_TextParse(&highScoreText, dynTextBuf, highScoreTextBuf);

	// optimize dynamic text
	C2D_TextOptimize(&scoreText);
	C2D_TextOptimize(&highScoreText);

	// draw the dynamic text
	C2D_DrawText(&scoreText, C2D_AlignCenter, 200.0f, 100.0f, 0.5f, 0.8f, 0.8f, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
	C2D_DrawText(&highScoreText, C2D_AlignCenter, 200.0f, 120.0f, 0.5f, 0.8f, 0.8f, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
}

void drawFood()
{
	if (foodEaten) {
		randomFoodPos(foodX, foodY);
		foodEaten = false;
	}

	C2D_DrawRectSolid(foodX * GRID_UNIT_SIZE, foodY * GRID_UNIT_SIZE, 0.0f, 10, 10, FOOD_COLOR);
}

void randomFoodPos(int& x, int& y)
{
	int MAX_X = gridX - 2;
	int MAX_Y = gridY - 2;
	int MIN = 1;

	srand(time(NULL));
	x = MIN + rand() % (MAX_X - MIN);
	y = MIN + rand() % (MAX_Y - MIN);
}

void checkBounds()
{
	// check if the snake has left the bounds of the screen
	if (posX == 0 || posX == gridX - 1 || posY == 0 || posY == gridY - 1) 
	{
		gameOver = true;
		if (score > highScore) highScore = score;
	}

	// check if the snake is overlapping the food
	if (posX == foodX && posY == foodY)
	{
		foodEaten = true;
		score++;
	}
}