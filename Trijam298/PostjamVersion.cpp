#include "global.h"
#include <unordered_set>
#include <string>

static Image collisionI;
static Texture2D background;
static Texture2D player;
static Texture2D door;
static Texture2D fanb;
static Texture2D fanbd;
static Texture2D fan;
static json entities;
static int level;
static int scrTop;
static int scrLeft;
static int px;
static int py;
static float vy;
static float vx;
static float ppy;
static float stam;
static float stammax;
static std::unordered_set<std::string> lastTriggeredFans;
static std::unordered_set<std::string> lastTriggeredButtons;
static std::unordered_set<std::string> triggeredFans;
static std::unordered_set<std::string> triggeredButtons;

static struct Particle {
	float r;
	int x;
	int sy;
	float y;
	float T;
};

static std::vector<Particle> particles;
static std::vector<Particle> lastParticles;

static void RandomizeParticle(Particle &p) {
	p.r = (float)GetRandomValue(0, 360) / 180.f * PI;
	p.y = p.sy;
	p.T = GetRandomValue(0, 10) / 6.f;
}

static void AddParticles(int sx, int sy) {
	for (int i = 0; i < 12; i++) {
		Particle p;
		p.x = sx;
		p.sy = sy;
		RandomizeParticle(p);
		particles.push_back(p);
	}
}

static const int mult = 0;

static bool IncrementLevel() {
	level++;
	lastTriggeredButtons = triggeredButtons;
	lastTriggeredFans = triggeredFans;
	lastParticles = particles;
	for (const auto &tl : entities["MapTL"]) {
		if (tl["customFields"]["id"] != level)
			continue;
		scrLeft = tl["x"];
		scrTop = tl["y"];

		for (const auto &p : entities["Player"]) {
			if (p["customFields"]["id"] != level)
				continue;
			px = p["x"];
			py = p["y"];
			stammax = stam = p["customFields"]["stamina"];
			PlaySound(SND_COMBO);
			return false;
		}
		break;
	}
	PlaySound(SND_REPAIR);
	return true;
}

static void RestartLevel() {
	PlaySound(SND_EXPLOSION);
	triggeredButtons = lastTriggeredButtons;
	triggeredFans = lastTriggeredFans;
	particles = lastParticles;
	for (const auto &p : entities["Player"]) {
		if (p["customFields"]["id"] != level)
			continue;
		px = p["x"];
		py = p["y"];
		stammax = stam = p["customFields"]["stamina"];
		return;
	}
}

static int GetTile(int p) {
	return p / 8;
}

static Color GetCollisionColor(int tx, int ty) {
	return GetImageColor(collisionI, tx, ty);
}

static bool IsTrampTile(int tx, int ty) {
	return GetCollisionColor(tx, ty).g == 255;
}

static bool IsFullTile(int tx, int ty) {
	return GetCollisionColor(tx, ty).b == 255 || IsTrampTile(tx, ty);
}
static bool IsTopTile(int tx, int ty) {
	return IsFullTile(tx, ty) || (GetCollisionColor(tx, ty).r == 255);
}
static bool IsOnGround() {
	bool onGround = IsFullTile(GetTile(px + 1), GetTile(py + 8)) || IsFullTile(GetTile(px + 6), GetTile(py + 8))
		|| (py % 8 == 0
			&& (IsTopTile(GetTile(px + 1), GetTile(py + 8)) || IsTopTile(GetTile(px + 6), GetTile(py + 8))));

	return onGround;
}

static void DrawParticles() {
	for (auto &p : particles) {
		p.T -= GetFrameTime();
		if (p.T > 0)
			continue;
		p.y--;
		p.r += GetFrameTime() * 0.9f;
		float x = p.x + cosf(p.r) * 8;
		if (IsFullTile(GetTile(x), GetTile(p.y))) {
			RandomizeParticle(p);
			continue;
		}
		DrawRectangle((int)x, (int)p.y, 1, 1, GRAY);
	}
}

static void TryMove(int dx, int dy) {
	if (dx > 0) {
		while (dx-- != 0) {
			if (IsTrampTile(GetTile(px + 7), GetTile(py + 1)) || IsTrampTile(GetTile(px + 7), GetTile(py + 7))) {
				vx = -10;
				vy -= 1;
				PlaySound(SND_FIRE);
			}
			else if (!IsFullTile(GetTile(px + 7), GetTile(py + 1)) && !IsFullTile(GetTile(px + 7), GetTile(py + 7))) {
				px++;
			}
			else {
				if (vx > 0)
					vx = 0;
			}
		}
	}
	else {
		while (dx++ != 0) {
			if (IsTrampTile(GetTile(px), GetTile(py + 1)) || IsTrampTile(GetTile(px), GetTile(py + 1))) {
				vx = 10;
				vy -= 1;
				PlaySound(SND_FIRE);
			}
			else if (!IsFullTile(GetTile(px), GetTile(py + 1)) && !IsFullTile(GetTile(px), GetTile(py + 1))) {
				px--;
			}
			else {
				if (vx > 0)
					vx = 0;
			}
		}
	}
	if (dy > 0) {
		while (dy-- != 0) {
			bool onGround = IsFullTile(GetTile(px + 1), GetTile(py + 8)) || IsFullTile(GetTile(px + 6), GetTile(py + 8))
				|| (py % 8 == 0
					&& (IsTopTile(GetTile(px + 1), GetTile(py + 8)) || IsTopTile(GetTile(px + 6), GetTile(py + 8))));
			if (!onGround)
				py++;
		}
	}
	else {
		while (dy++ != 0) {
			if (!IsFullTile(GetTile(px + 1), GetTile(py)) && !IsFullTile(GetTile(px + 6), GetTile(py)))
				py--;
		}
	}
}

static bool ProcessDoor() {
	Rectangle p;
	p.x = px + 1;
	p.y = py + 1;
	p.width = 6;
	p.height = 7;
	for (const auto &d : entities["Door"]) {
		Rectangle D;
		D.x = (int)d["x"] + 3;
		D.y = (int)d["y"] + 3;
		D.width = 2;
		D.height = 5;
		int t = d["customFields"]["id"];
		if (t == level && CheckCollisionRecs(p, D)) {
			return IncrementLevel();
		}
	}
	return false;
}

static void OverFan() {
	Rectangle p;
	p.x = px + 1;
	p.y = py + 1;
	p.width = 6;
	p.height = 7;
	bool overfan = false;
	for (const auto &f : entities["Fan"]) {
		if (!triggeredFans.contains(f["iid"]))
			continue;
		for (const auto &b : entities["FanBody"]) {
			if (b["iid"] != f["customFields"]["body"]["entityIid"])
				continue;
			Rectangle D;
			D.x = b["x"];
			D.y = b["y"];
			D.width = 16;
			D.height = b["height"];
			if (CheckCollisionRecs(p, D)) {
				vy = -2;
				overfan = true;
			}
		}
	}
	if (overfan) {
		PlaySound(SND_PROGRESS);
	}
	else {
		StopSound(SND_PROGRESS);
	}
}

static void KillTile() {
	Rectangle p;
	p.x = px + 2;
	p.y = py + 2;
	p.width = 4;
	p.height = 5;
	for (const auto &b : entities["Kill"]) {
		Rectangle D;
		D.x = b["x"];
		D.y = b["y"];
		D.width = b["width"];
		D.height = b["height"];
		if (CheckCollisionRecs(p, D)) {
			RestartLevel();
		}
	}
}

static void FanButton() {
	Rectangle p;
	p.x = px + 1;
	p.y = py + 1;
	p.width = 6;
	p.height = 7;
	//5, 2
	for (const auto &b : entities["FanButton"]) {
		if (triggeredButtons.contains(b["iid"]))
			continue;
		Rectangle D;
		D.x = (int)b["x"] + 5;
		D.y = (int)b["y"] + 2;
		D.width = 4;
		D.height = 2;
		if (!CheckCollisionRecs(p, D))
			continue;
		std::string fid = b["customFields"]["fan"]["entityIid"];
		triggeredButtons.insert(b["iid"]);
		triggeredFans.insert(fid);
		PlaySound(SND_DETECTION);
		for (const auto &f : entities["Fan"]) {
			if (f["iid"] != fid)
				continue;
			AddParticles((int)f["x"] + 8, f["y"]);
			break;
		}
	}
}

static bool GameOver() {
	StopSound(SND_MUSIC);
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(BLACK);
		DrawText("You Won!", (800 - MeasureText("You Won!", 60)) / 2, 100, 60, WHITE);
		DrawKeybindBar("", "[Enter] Play Again");
		EndDrawing();

		if (IsKeyPressed(KEY_ENTER))
			return true;
	}
	return false;
}

void PostjamPreloadAssets() {
	collisionI = LoadImage("ldtk/map/simplified/Level_0/Collision-int.png");
	background = LoadTexture("ldtk/map/simplified/Level_0/_composite.png");
	player = LoadTexture("player.png");
	door = LoadTexture("door.png");
	fanb = LoadTexture("fanbutton.png");
	fanbd = LoadTexture("fanbuttond.png");
	fan = LoadTexture("fan.png");
	char *fileText = LoadFileText("ldtk/map/simplified/Level_0/data.json");
	entities = json::parse(fileText)["entities"];
	UnloadFileText(fileText);
}

bool PostjamRunGame() {
	int fadein = 0;
	bool restart = false;


	level = -1;
	lastTriggeredButtons = {};
	triggeredFans = {};
	triggeredButtons = {};
	lastTriggeredFans = {};
	particles = {};
	lastParticles = {};
	vy = 0;
	vx = 0;

	IncrementLevel();

	PlaySound(SND_START);
	//DoFadeOutAnimation();

	while (!WindowShouldClose()) {
		SetSoundVolume(GetSound(SND_MUSIC), 4.f);
		PlaySound(SND_MUSIC);
		if (IsKeyDown(KEY_LEFT))
			TryMove(-1, 0);
		if (IsKeyDown(KEY_RIGHT))
			TryMove(1, 0);

		if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT))
			stam -= 1.f / 30.f;

		vy += 0.2f;
		vy *= 0.9f;

		vx *= 0.9f;
		TryMove(vx, 0);

		if (vy > 0 && IsOnGround())
			vy = 0;
		if (IsOnGround() && IsKeyPressed(KEY_UP)) {
			vy = -2.7f;
			stam -= 0.3f;
			PlaySound(SND_FIRE);
		}

		ppy += vy;
		while (ppy >= 1) {
			TryMove(0, 1);
			ppy -= 1;
		}
		while (ppy <= -1) {
			TryMove(0, -1);
			ppy += 1;
		}

		if (ProcessDoor()) {
			restart = GameOver();
			goto END;
		}

		if (stam <= 0) {
			RestartLevel();
		}

		FanButton();
		OverFan();
		KillTile();

		BeginDrawing();

		ClearBackground(BLACK);

		Camera2D c{ 0 };
		c.zoom = 6;
		c.target.x = scrLeft;
		c.target.y = scrTop;
		c.rotation = 0;

		BeginMode2D(c);

		DrawTexture(background, 0, 0, WHITE);

		DrawParticles();

		for (const auto &d : entities["Fan"]) {
			//DrawRectangle(d["x"], d["y"], d["width"], d["height"], Fade(RED, 0.5f));
			DrawTexture(fan, d["x"], d["y"], triggeredFans.contains(d["iid"]) ? WHITE : GRAY);
		}

		for (const auto &d : entities["FanBody"]) {
			if (!triggeredFans.contains(d["customFields"]["fan"]["entityIid"]))
				continue;
			//DrawRectangle(d["x"], d["y"], d["width"], d["height"], Fade(ORANGE, 0.5f));
		}

		for (const auto &d : entities["FanButton"]) {
			DrawTexture(triggeredButtons.contains(d["iid"]) ? fanbd : fanb, d["x"], d["y"], WHITE);
		}

		for (const auto &d : entities["Door"]) {
			DrawTexture(door, d["x"], d["y"], WHITE);
		}

		DrawTexture(player, px, py, WHITE);

		EndMode2D();
		{
			Color c = WHITE;
			c.g *= stam / stammax;
			c.b *= stam / stammax;
			c.a = (int)(255 * (1.f - (stam / stammax)));
			c.a /= 2;
			c.a += 128;
			DrawRectangle(0, SCRHEI - 32 + 6, SCRWID * stam / stammax, 32 - 6, c);
		}
		// DrawText(TextFormat("%.1f", stammax - stam), 0, SCRHEI - 32 - 20, 20, WHITE);

		if (level == 0)
			DrawKeybindBar("[Left] Left [Right] Right", "[Up] Jump", false);

		DoFadeInAnimation(fadein);

		EndDrawing();
	}

END:
	SaveGlobState();

	StopSound(SND_MUSIC);
	//UnloadImage(collisionI);
	//UnloadTexture(background);
	//UnloadTexture(fanb);
	//UnloadTexture(player);
	//UnloadTexture(fanbd);
	//UnloadTexture(fan);

	return restart;
}