#include "main.h"

C3D_RenderTarget* top;
u32 COLOR_CLEAR = C2D_Color32(0xFF, 0xD8, 0xB0, 0x68);
u32 COLOR_RED = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
u32 kDownOld = 0;
extern short sDirection;
extern bool gameOver;
extern LightEvent s_event;
static const char* OPUS_PATH = "romfs:/Auvic_Consumed.opus";
static const int THREAD_AFFINITY = -1;
static const int THREAD_STACK_SZ = 32 * 1024;
extern volatile bool s_quit;
int frameCount = 0;

void render(void)
{
	// Render the scene
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(top, COLOR_CLEAR);
	C2D_SceneBegin(top);
	gameRender(); // call the render method in game.cpp, used for updating lerp
	C3D_FrameEnd(0);
}

int main(void)
{
	// Initialize stuff
	romfsInit();
	ndspInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	consoleInit(GFX_BOTTOM, NULL);
	osSetSpeedupEnable(true);
	LightEvent_Init(&s_event, RESET_ONESHOT);

	int error = 0;
	OggOpusFile* opusFile = op_open_file(OPUS_PATH, &error);
	if (error) {
		printf("Failed to open file: error %d (%s)\n", error,
			opusStrError(error));
	}

	// Attempt audioInit
	if (!audioInit()) {
		printf("Failed to initialise audio\n");

		C2D_Fini();
		C3D_Fini();
		gfxExit();
		ndspExit();
		romfsExit();
		return EXIT_FAILURE;
	}

	// Set the ndsp sound frame callback which signals our audioThread
	ndspSetCallback(audioCallback, NULL);

	// Spawn audio thread

	// Set the thread priority to the main thread's priority ...
	int32_t priority = 0x30;
	svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
	// ... then subtract 1, as lower number => higher actual priority ...
	priority -= 1;
	// ... finally, clamp it between 0x18 and 0x3F to guarantee that it's valid.
	priority = priority < 0x18 ? 0x18 : priority;
	priority = priority > 0x3F ? 0x3F : priority;

	// Start the thread, passing our opusFile as an argument.
	const Thread threadId = threadCreate(audioThread, opusFile,
		THREAD_STACK_SZ, priority,
		THREAD_AFFINITY, false);
	printf("Created audio thread %p\n", threadId);

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

		if(frameCount % FPS == 0)
			render();

		frameCount++;
	}

	// Signal audio thread to quit
	s_quit = true;
	LightEvent_Signal(&s_event);

	// Free the audio thread
	threadJoin(threadId, UINT64_MAX);
	threadFree(threadId);

	// Cleanup audio things and de-init platform features
	audioExit();
	ndspExit();
	op_free(opusFile);

	gameExit();
	C2D_Fini();
	C3D_Fini();
	romfsExit();
	gfxExit();
	return EXIT_SUCCESS;
}
