#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#define PLAY_ADD_GAMEOBJECT_MEMBERS float sineOffset = 0.0f;
#include "Play.h"

int DISPLAY_WIDTH = 320;
int DISPLAY_HEIGHT = 180;
int DISPLAY_SCALE = 4;

constexpr int wrapBorderSize = 10;

enum class FishingState
{
	STATE_NULL = 0,
	STATE_FISHING,
	STATE_REEL,
	STATE_CATCHING,
	STATE_CAUGHT,
	STATE_LOST
};

enum class PlayState
{
	STATE_MENU = 0,
	STATE_APPEAR,
	STATE_PLAY,
	STATE_OUTCOME,
	STATE_RESTART
};

struct GameState
{
	float timer = 0;
	float t = 0;
	int score = 0;
	int fishCounter = 0;
	int fishPoints = 0;
	int spriteId = 0;
	int winPoints = 0;
	int losePoints = 0;
	int miniGameCountdown = 60;
	int currentLevel = 0;
	int pointsNeeded = -1;
	int caughtFish = -1;
	const int BASS_SCORE = 1;
	const int DAB_SCORE = 2;
	const int GILL_SCORE = 3;
	const int GOLDEN_SCORE = 5;
	const int FISH_UNLOCK_ONE = 2;
	const int FISH_UNLOCK_TWO = 6;
	const int FISH_UNLOCK_THREE = 10;
	const int NUMBER_OF_UNLOCKS = 3;
	bool fishSpawnedBass = false;
	bool fishSpawnedUnlock1 = false;
	bool fishSpawnedUnlock2 = false;
	bool fishSpawnedUnlock3 = false;
	bool hasCaught = false;
	bool hasCaughtGolden = false;
	bool canFish = true;
	FishingState fishingState = FishingState::STATE_NULL;
	PlayState playState = PlayState::STATE_MENU;

};

GameState gameState;

enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_BASS,
	TYPE_DAB,
	TYPE_GILL,
	TYPE_GOLDEN,
	TYPE_ROD,
	TYPE_BAR_UI,
	TYPE_FILL_UI,
	TYPE_FISH_UI,
	TYPE_LETTER_UI,
	TYPE_PROGRESS_UI,
	TYPE_BAG
};

// function prototypes
void ScreenWrap(GameObject& obj, Vector2f origin);
void SpawnFish(GameObjectType TYPE, int count, const char* sprite_left, const char* sprite_right);
void UpdateFish(GameObjectType TYPE);
void UpdateFishingState();
void SpawnRod();
void UpdateRod();
void PlayerControls();
void SpawnFishingUI();
void UpdateFishingUI(const char* fill, int collisionRadius);
void UpdateFillUI();
void PlayerControlsUI(int fillHeightUp, int fillHeightDown);
void UpdateFishUI();
void WinFish();
void LoseFish();
void SpawnProgressBarUI();
void UpdateProgressBarUI();
void InitialiseFishManager();
void UpdateFishManager();
void UpdatePlayState();
void SpawnLetterUI();
void UpdateLetterUI(const char* letter);
void EndGameManager();
void WinGame();
void LoseGame();
void DestroyFish(int id_fish);
void DestroyLetterUI();
void PointsUntilNextUnlock();

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\underwater.png");
	Play::CentreAllSpriteOrigins();
	Play::StartAudioLoop("music");
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	gameState.timer += elapsedTime;
	Play::DrawBackground();
	UpdateFishManager();
	UpdatePlayState();
	UpdateFishingState();
	PointsUntilNextUnlock();
	Play::DrawFontText("32px", "SCORE: " + std::to_string(gameState.score), { 285, 15 }, Play::CENTRE);

	if (Play::KeyDown(VK_F2)) 
	{
		gameState.score = 10;
	}
	if (Play::KeyDown(VK_SHIFT)) 
	{
		Play::StopAudioLoop("fishing_reel");
		Play::StopAudioLoop("pulling_fish");
		gameState.playState = PlayState::STATE_RESTART;
		gameState.fishingState = FishingState::STATE_NULL;
	}

	Play::PresentDrawingBuffer();
	return Play::KeyDown(VK_ESCAPE);
}

void ScreenWrap(GameObject& obj, Vector2f origin)
{
	if (obj.pos.x - origin.x - wrapBorderSize > DISPLAY_WIDTH)
	{
		obj.pos.x = 0.0f - wrapBorderSize + origin.x;
	}
	else if (obj.pos.x + origin.x + wrapBorderSize < 0)
	{
		obj.pos.x = DISPLAY_WIDTH + wrapBorderSize - origin.x;
	}
}

void SpawnFish(GameObjectType TYPE, int count, const char* sprite_left, const char* sprite_right)
{
	for (int i = 0; i < count; i++)
	{
		int myFishId = Play::CreateGameObject(TYPE, { rand() % DISPLAY_WIDTH, rand() % DISPLAY_HEIGHT }, 5, sprite_left);
		GameObject& obj_fish = Play::GetGameObject(myFishId);

		gameState.fishCounter++;

		// sine offset 0 -> 2 pi
		obj_fish.sineOffset = ((rand() % 1000) / 1000.0f) * 2.0 * PLAY_PI;

		// (0 -> 999) / 1000.0
		float randomRatio = (rand() % 1000) / 1000.0f;

		obj_fish.velocity.x = 0.3f + (randomRatio * 0.5f);

		// coin flip left or right 
		if ((rand() % 100) < 50) obj_fish.velocity.x = -obj_fish.velocity.x;

		// rotate sprite
		if (obj_fish.velocity.x >= 0)
		{
			Play::SetSprite(obj_fish, sprite_right, 0);
			obj_fish.rotation = 0.8;
		}
		else
		{
			obj_fish.rotation = -0.8;
		}
	} 
}
void UpdateFish(GameObjectType TYPE)
{
	std::vector<int> vFishIds = Play::CollectGameObjectIDsByType(TYPE);
	GameObject& obj_rod = Play::GetGameObjectByType(TYPE_ROD);

	// sine wave variables
	float sineSpeed = 1.5f;
	float sineMovement = 0.2f;

	for (int id_fish : vFishIds)
	{
		GameObject& obj_fish = Play::GetGameObject(id_fish);

		Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_fish.spriteId);
		ScreenWrap(obj_fish, origin);

		Play::DrawObjectRotated(obj_fish);

		// sine wave
		obj_fish.velocity.y = sineMovement * sin(gameState.timer * sineSpeed + obj_fish.sineOffset);

		Play::UpdateGameObject(obj_fish);

		if (Play::IsColliding(obj_fish, obj_rod) && gameState.canFish == true)
		{
			gameState.caughtFish = id_fish;
			obj_rod.velocity.y = -0.5;
			gameState.canFish = false;
			Play::StopAudioLoop("fishing_reel");
			Play::StartAudioLoop("pulling_fish");
			gameState.fishingState = FishingState::STATE_REEL;
		}

		if (id_fish == gameState.caughtFish) 
		{
			obj_fish.pos = obj_rod.pos;
		}
	}
}

void DestroyFish(int id_obj) 
{
	Play::DestroyGameObject(id_obj);
	gameState.fishCounter--;
	gameState.caughtFish = -1;
}

void InitialiseFishManager()
{
	if (gameState.score == 0 && gameState.fishSpawnedBass == false) 
	{
		SpawnFish(TYPE_BASS, 6, "fish_bass_left", "fish_bass_right");
		gameState.fishSpawnedBass = true;
	}
	if (gameState.score >= gameState.FISH_UNLOCK_ONE && gameState.fishSpawnedUnlock1 == false)
	{
		SpawnFish(TYPE_DAB, 3, "fish_dab_left", "fish_dab_right");
		gameState.fishSpawnedUnlock1 = true;
	}
	if (gameState.score >= gameState.FISH_UNLOCK_TWO && gameState.fishSpawnedUnlock2 == false)
	{
		SpawnFish(TYPE_GILL, 3, "fish_bluegill_left", "fish_bluegill_right");
		gameState.fishSpawnedUnlock2 = true;
	}
	if (gameState.score >= gameState.FISH_UNLOCK_THREE && gameState.fishSpawnedUnlock3 == false)
	{
		SpawnFish(TYPE_GOLDEN, 1, "fish_golden_left", "fish_golden_right");
		gameState.fishSpawnedUnlock3 = true;
	}
}

void UpdateFishManager()
{
	UpdateFish(TYPE_BASS);

	if (gameState.score >= gameState.FISH_UNLOCK_ONE)
	{
		UpdateFish(TYPE_DAB);
	}
	if (gameState.score >= gameState.FISH_UNLOCK_TWO)
	{
		UpdateFish(TYPE_GILL);
	}
	if (gameState.score >= gameState.FISH_UNLOCK_THREE)
	{
		UpdateFish(TYPE_GOLDEN);
	}
}

void PlayerControls()
{
	GameObject& obj_rod = Play::GetGameObjectByType(TYPE_ROD);

	if (gameState.fishingState == FishingState::STATE_FISHING)
	{
		constexpr int GROUND = 175;

		// rod movement
		if (Play::KeyDown(VK_UP))
		{
			obj_rod.velocity.y = -1;
			Play::StartAudioLoop("fishing_reel");
		}

		else if (Play::KeyDown(VK_DOWN) && obj_rod.pos.y < GROUND)
		{
			obj_rod.velocity.y = 2;
			Play::StartAudioLoop("fishing_reel");
		}
		// stop rod
		else
		{
			// down
			if (obj_rod.pos.y >= GROUND)
			{
				obj_rod.velocity.y = 0;
				Play::StopAudioLoop("fishing_reel");
			}
			// up
			else if (obj_rod.pos.y <= -10)
			{
				obj_rod.velocity.y = 0;
				Play::StopAudioLoop("fishing_reel");
			}
		}
		if (obj_rod.pos.y == 0 && obj_rod.velocity.y == 2) 
		{
			Play::PlayAudio("hits_water");
		}
	}
}
void SpawnRod()
{
	Play::CreateGameObject(TYPE_ROD, { 160, -10 }, 5, "hook");
	Play::MoveSpriteOrigin("hook", 0, 2);
}

void UpdateRod()
{
	GameObject& obj_rod = Play::GetGameObjectByType(TYPE_ROD);
	Play::DrawObject(obj_rod);

	Play::DrawLine({ obj_rod.pos.x, -10 }, obj_rod.pos, Play::cWhite);
	Play::UpdateGameObject(obj_rod);
}

void SpawnProgressBarUI()
{
	Play::CreateGameObject(TYPE_PROGRESS_UI, { 20, 45 }, 10, "progress_bar_empty");
}

void UpdateProgressBarUI()
{
	GameObject& obj_progress_bar = Play::GetGameObjectByType(TYPE_PROGRESS_UI);

	if (gameState.score >= gameState.FISH_UNLOCK_THREE)
	{
		Play::SetSprite(obj_progress_bar, "progress_bar_fill_full", 0);
	}
	else if (gameState.score >= gameState.FISH_UNLOCK_TWO)
	{
		Play::SetSprite(obj_progress_bar, "progress_bar_fill_second", 0);
	}
	else if (gameState.score >= gameState.FISH_UNLOCK_ONE)
	{
		Play::SetSprite(obj_progress_bar, "progress_bar_fill_first", 0);
	}

	Play::DrawObject(obj_progress_bar);
	Play::UpdateGameObject(obj_progress_bar);
}

void SpawnFishingUI()
{
	Play::CreateGameObject(TYPE_BAR_UI, { 80, 90 }, 20, "bar");
	Play::CreateGameObject(TYPE_FILL_UI, { 80, 90 }, 15, "fill_easy");
	Play::CreateGameObject(TYPE_FISH_UI, { 80, 90 }, 2, "fish_ui");
}

void UpdateFishingUI(const char* fill, int collisionRadius)
{
	GameObject& obj_bar = Play::GetGameObjectByType(TYPE_BAR_UI);
	GameObject& obj_fill = Play::GetGameObjectByType(TYPE_FILL_UI);
	GameObject& obj_fish_ui = Play::GetGameObjectByType(TYPE_FISH_UI);

	Play::DrawObject(obj_bar);
	Play::DrawObject(obj_fill);
	Play::DrawObject(obj_fish_ui);

	Play::SetSprite(obj_fill, fill, 0);
	obj_fill.radius = collisionRadius;

	constexpr int POINTS_TO_CATCH_FISH = 300;
	constexpr int POINTS_TO_LOSE_FISH = 300;

	if (Play::IsColliding(obj_fish_ui, obj_fill))
	{
		gameState.winPoints++;
	}
	else if (!Play::IsColliding(obj_fish_ui, obj_fill))
	{
		gameState.losePoints++;
	}

	if (gameState.winPoints >= POINTS_TO_CATCH_FISH && gameState.hasCaught == false)
	{
		Play::PlayAudio("catch_fish");
		gameState.hasCaught = true;
		gameState.score += gameState.fishPoints;
		gameState.fishingState = FishingState::STATE_CAUGHT;
	}
	if (gameState.losePoints >= POINTS_TO_LOSE_FISH)
	{
		Play::PlayAudio("lost_fish");
		gameState.fishingState = FishingState::STATE_LOST;
	}
	Play::UpdateGameObject(obj_bar);
	Play::UpdateGameObject(obj_fill);
	Play::UpdateGameObject(obj_fish_ui);
}

void UpdateFishUI()
{
	GameObject& obj_fish_ui = Play::GetGameObjectByType(TYPE_FISH_UI);

	constexpr int BAR_HEIGHT_TOP = 40;
	constexpr int BAR_HEIGHT_BOTTOM = 140;
	constexpr int DIVISOR = 20;
	gameState.miniGameCountdown--;

	// wait til reach then reset t
	if (gameState.miniGameCountdown <= 0) 
	{
		// 0 to 1
		gameState.t = (float)(rand() % 1000) / 1000.0f;
		gameState.miniGameCountdown = 60;
	}

	// lerp between random point 0 -> 1
	// 0 t is 40, 1t is 140
	int targetPos = BAR_HEIGHT_TOP + gameState.t * (BAR_HEIGHT_BOTTOM - BAR_HEIGHT_TOP);
	int oldPos = obj_fish_ui.pos.y;
	// subtract current pos from target & add fraction of difference to current pos
	// bigger divisor slower movement
	obj_fish_ui.pos.y += (targetPos - oldPos) / DIVISOR;

}

void UpdateFillUI() 
{
	if (gameState.fishPoints <= 1)
	{
		UpdateFishingUI("fill_easy", 15);
		PlayerControlsUI(60, 120);
	}
	if (gameState.fishPoints == gameState.DAB_SCORE)
	{
		UpdateFishingUI("fill_medium", 12);
		PlayerControlsUI(56, 124);
	}
	if (gameState.fishPoints == gameState.GILL_SCORE)
	{
		UpdateFishingUI("fill_hard", 9);
		PlayerControlsUI(53, 127);
	}
	if (gameState.fishPoints == gameState.GOLDEN_SCORE)
	{
		UpdateFishingUI("fill_golden", 6);
		PlayerControlsUI(50, 130);
	}
}

void PlayerControlsUI(int fillHeightUp, int fillHeightDown)
{
	GameObject& obj_fill = Play::GetGameObjectByType(TYPE_FILL_UI);

	if (Play::KeyPressed(VK_UP))
	{
		obj_fill.velocity.y = -1.0;
		Play::PlayAudio("fishing_reel");
	}

	else if (Play::KeyPressed(VK_DOWN))
	{
		obj_fill.velocity.y = 1.0;
		Play::PlayAudio("fishing_reel");
	}
	else
	{
		if (obj_fill.pos.y >= fillHeightDown)
		{
			obj_fill.pos.y = fillHeightDown;
		}

		else if (obj_fill.pos.y <= fillHeightUp)
		{
			obj_fill.pos.y = fillHeightUp;
		}
	}

}

void SpawnLetterUI()
{
	Play::CreateGameObject(TYPE_LETTER_UI, { 160, 90 }, 5, "catch_bass");
}

void UpdateLetterUI(const char* letter) 
{
	GameObject& obj_letter = Play::GetGameObjectByType(TYPE_LETTER_UI);

	Play::SetSprite(obj_letter, letter, 0);
	Play::DrawObject(obj_letter);
	Play::UpdateGameObject(obj_letter);

}

void DestroyLetterUI() 
{
	Play::DestroyGameObjectsByType(TYPE_LETTER_UI);

}
void WinFish()
{
	SpawnLetterUI();

	if (gameState.fishPoints == 1)
	{
		UpdateLetterUI("catch_bass");
	}
	if (gameState.fishPoints == gameState.DAB_SCORE)
	{
		UpdateLetterUI("catch_dab");
	}
	if (gameState.fishPoints == gameState.GILL_SCORE)
	{
		UpdateLetterUI("catch_blue_gill");
	}
	if (gameState.fishPoints == gameState.GOLDEN_SCORE)
	{
		UpdateLetterUI("catch_golden_tench");
	}

	if (Play::KeyPressed(VK_SPACE))
	{
		DestroyLetterUI();
		gameState.fishingState = FishingState::STATE_FISHING;
		gameState.playState = PlayState::STATE_PLAY;
	}
}

void LoseFish()
{
	SpawnLetterUI();
	UpdateLetterUI("letter_miss");
	if (Play::KeyPressed(VK_SPACE))
	{
		gameState.fishingState = FishingState::STATE_FISHING;
		gameState.playState = PlayState::STATE_PLAY;
	}
}

void EndGameManager() 
{
	if (gameState.hasCaughtGolden) 
	{
		WinGame();
	}

	if (gameState.fishCounter == 0) 
	{
		LoseGame();
	}
}

void WinGame() 
{
	if (Play::KeyPressed(VK_SPACE)) 
	{
		gameState.playState = PlayState::STATE_RESTART;
		gameState.fishingState = FishingState::STATE_NULL;
	}
}

void LoseGame() 
{
	gameState.playState = PlayState::STATE_RESTART;
	gameState.fishingState = FishingState::STATE_NULL;
}

void PointsUntilNextUnlock() 
{
	if (gameState.currentLevel < gameState.NUMBER_OF_UNLOCKS) 
	{
		int UnlockPoints[] = { gameState.FISH_UNLOCK_ONE, gameState.FISH_UNLOCK_TWO, gameState.FISH_UNLOCK_THREE };

		int scoreNeededNextLvl = UnlockPoints[gameState.currentLevel];

		if (gameState.score >= scoreNeededNextLvl)
		{
			++gameState.currentLevel;
		}

		gameState.pointsNeeded = scoreNeededNextLvl - gameState.score;
	}
	else 
	{
		gameState.pointsNeeded = 0;
	}
}

void PointsUntilUnlockUI()
{
	if (gameState.currentLevel < gameState.NUMBER_OF_UNLOCKS) 
	{
		Play::DrawFontText("32px", std::to_string(gameState.pointsNeeded) + " POINTS UNTIL A NEW FISH UNLOCKS ", { 170, 160 }, Play::CENTRE);
	}
	else 
	{
		Play::DrawFontText("32px", "CATCH THE GOLDEN FISH TO WIN", { 170, 160 }, Play::CENTRE);
	}
}

void UpdatePlayState()
{
	switch (gameState.playState)
	{
	case PlayState::STATE_MENU:
		Play::DrawFontText("32px", "SCORE: " + std::to_string(gameState.score), { 285, 15 }, Play::CENTRE);
		Play::DrawFontText("32px", "CATCH THE GOLDEN FISH TO WIN", { 170, 160 }, Play::CENTRE);
		SpawnLetterUI();
		UpdateLetterUI("letter_menu");
		if (Play::KeyPressed(VK_SPACE)) 
		{
			gameState.playState = PlayState::STATE_APPEAR;
			gameState.fishingState = FishingState::STATE_FISHING;
		}
		break;
	case PlayState::STATE_APPEAR:
		InitialiseFishManager();
		SpawnRod();
		SpawnProgressBarUI();
		gameState.playState = PlayState::STATE_PLAY;
		break;
	case PlayState::STATE_PLAY:
		UpdateProgressBarUI();
		break;
	case PlayState::STATE_OUTCOME:
		Play::StopAudioLoop("fishing_reel");
		Play::DestroyGameObjectsByType(TYPE_BAR_UI);
		Play::DestroyGameObjectsByType(TYPE_FILL_UI);
		Play::DestroyGameObjectsByType(TYPE_FISH_UI);
		UpdateProgressBarUI();
		EndGameManager();
		gameState.winPoints = 0;
		gameState.losePoints = 0;
		break;
	case PlayState::STATE_RESTART:
		Play::DestroyGameObjectsByType(TYPE_BASS);
		Play::DestroyGameObjectsByType(TYPE_DAB);
		Play::DestroyGameObjectsByType(TYPE_GILL);
		Play::DestroyGameObjectsByType(TYPE_GOLDEN);
		gameState.score = 0;
		gameState.timer = 0;
		gameState.fishSpawnedBass = false;
		gameState.fishSpawnedUnlock1 = false;
		gameState.fishSpawnedUnlock2 = false;
		gameState.fishSpawnedUnlock3 = false;
		gameState.playState = PlayState::STATE_MENU;
		break;

	}
}

void UpdateFishingState()
{
	switch (gameState.fishingState)
	{
	case FishingState::STATE_NULL:
		break;
	case FishingState::STATE_FISHING:
		gameState.hasCaught = false;
		gameState.hasCaughtGolden = false;
		UpdateRod();
		PlayerControls();
		PointsUntilUnlockUI();
		break;
	case FishingState::STATE_REEL:
	{
		GameObject& obj_rod = Play::GetGameObjectByType(TYPE_ROD);
		GameObject& obj_fish = Play::GetGameObject(gameState.caughtFish);

		if (obj_rod.pos.y <= -10)
		{
			Play::StopAudioLoop("pulling_fish");

			gameState.canFish = true;
			obj_rod.velocity.y = 0;

			switch (obj_fish.type)
			{
			case TYPE_BASS:
				gameState.fishPoints = gameState.BASS_SCORE;
				break;
			case TYPE_DAB:
				gameState.fishPoints = gameState.DAB_SCORE;
				break;
			case TYPE_GILL:
				gameState.fishPoints = gameState.GILL_SCORE;
				break;
			case TYPE_GOLDEN:
				gameState.fishPoints = gameState.GOLDEN_SCORE;
				gameState.hasCaughtGolden = true;
				break;
			}

			DestroyFish(gameState.caughtFish);
			gameState.fishingState = FishingState::STATE_CATCHING;
			SpawnFishingUI();
		}
		UpdateRod();
		PointsUntilUnlockUI();
	}
	break;
	case FishingState::STATE_CATCHING:
		UpdateFillUI();
		UpdateFishUI();
		Play::DrawFontText("32px", "KEEP THE ORANGE BAR OVER THE FISH", { 170, 160 }, Play::CENTRE);
		break;
	case FishingState::STATE_CAUGHT:
		gameState.playState = PlayState::STATE_OUTCOME;
		InitialiseFishManager();
		WinFish();
		break;
	case FishingState::STATE_LOST:
		gameState.playState = PlayState::STATE_OUTCOME;
		LoseFish();
		break;
	}
}
// Gets called once when the player quits the game 
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}

