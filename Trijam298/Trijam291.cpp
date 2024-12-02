#include "global.h"

// WHATEVER YOU DO, DO NOT ADD CLASSES PLEASE FOR THE LOVE OF GOD. OR, IF YOU DO, THINK ABOUT IT. THINK "DO I NEED THIS". THINK THAT AND THEN SAY /NO/!

#ifdef _DEBUG
// #define FORCE_EDITION 0
#endif

static bool updated = false;

void ProcessFlags(int sel) {
	updated = sel == 0 ? false : true;
}

bool PickFlags() {
#ifdef FORCE_EDITION
	ProcessFlags(FORCE_EDITION);
	return true;
#endif

	int sel = 0;
	Color overflow = BLACK;
	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_UP)) {
			sel--;
			PlaySound(SND_MENU);
			overflow = BLACK;
		} if (IsKeyPressed(KEY_DOWN)) {
			sel++;
			PlaySound(SND_MENU);
			overflow = BLACK;
		}

		if (sel > 1) {
			sel = 0;
			overflow = BLUE;
		}
		else if (sel < 0) {
			sel = 1;
			overflow = ORANGE;
		}

		if (IsKeyPressed(KEY_ENTER)) {
			ProcessFlags(sel);
			return true;
		}

		BeginDrawing();

		ClearBackground(BLACK);

		// TODO: Replace this.
		DrawText("Edition of Running Home:", 15, 15, 20, WHITE);

		DrawLine(15, 45, 395, 45, overflow);
		DrawLine(15, 125, 395, 125, overflow);
		//DrawLine(15, 165, 395, 165, overflow);

		if (sel == 0) DrawRectangle(15, 50, 380, 30, DARKGRAY);
		if (sel == 0) DrawRectangleLines(15, 50, 380, 30, WHITE);
		if (sel == 0) DrawText("Version of the game made\n\nduring the three hours\n\nof the Trijam.", 460, 50, 20, WHITE);
		DrawText("3-Hour Edition", 25, 55, 20, WHITE);

		if (sel == 1) DrawRectangle(15, 90, 380, 30, DARKGRAY);
		if (sel == 1) DrawRectangleLines(15, 90, 380, 30, WHITE);
		if (sel == 1) DrawText("Version of the game with\n\nbugfixes and new levels.", 460, 50, 20, WHITE);
		DrawText("Postjam Edition", 25, 95, 20, WHITE);

		DrawKeybindBar("[Up] [Down]", "[Enter] Select");

		EndDrawing();
	}
	return false;
}

bool TrijamRunGame();
bool PostjamRunGame();
void PreloadAssets();
void PostjamPreloadAssets();

int main() {
	/*{
		R r = RRead("save.dat");
		if (r.file) {
			SER_REV(r);
			SERIALIZE(r, globstate);
		}
		RClose(r);
	}*/

	InitWindow(SCRWID, SCRHEI, "Trijam 298");
	PreloadAssets();
	PostjamPreloadAssets();
	InitAudioDevice();
	LoadSounds();
	SetExitKey(0);

	SetTargetFPS(30);

	if (!PickFlags())
		goto END;

	while (updated ? PostjamRunGame() : TrijamRunGame());

END:
	CloseWindow();
}