#include "main.h"

C3D_RenderTarget* top;
u32 COLOR_CLEAR = C2D_Color32(0xFF, 0xD8, 0xB0, 0x68);
u32 COLOR_RED = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
u32 kDownOld = 0;
extern short sDirection;
int frame = 0;
bool gameOver = false;

void render(void)
{
	// Render the scene
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(top, COLOR_CLEAR);
	C2D_SceneBegin(top);
	gameRender(); // call the render method in game.cpp, used for updating lerp
	drawGrid();
	drawSnake();
	if (gameOver)
	{
		drawGameOver();
	}
	C3D_FrameEnd(0);
}

void sleep(int ms)
{
	while (ms--) gspWaitForVBlank();
}

int main(void)
{
	// Init Libs
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	consoleInit(GFX_BOTTOM, NULL);

	// init game.cpp
	gameInit();

	// Init grid
	initGrid(COLUMNS, ROWS);

	// Create screens
	top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	// call the initial render
	render();

	// Main loop
	while (aptMainLoop())
	{
		hidScanInput();
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		if (kDown != kDownOld)
		{
			if (!gameOver)
			{
				if (kDown & KEY_RIGHT) sDirection = RIGHT;
				if (kDown & KEY_LEFT) sDirection = LEFT;
				if (kDown & KEY_UP) sDirection = UP;
				if (kDown & KEY_DOWN) sDirection = DOWN;
			}
			else
			{
				if (kDown & KEY_A) gameReset();
			}
		}
		
		if (gameOver)
		{
			sDirection = -1;
		}

		kDownOld = kDown;

		if(frame % FPS == 0)
			render();

		frame++;
	}

	gameExit();
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}
