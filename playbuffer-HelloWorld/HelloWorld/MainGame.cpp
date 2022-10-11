#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 320;
int DISPLAY_HEIGHT = 180;
int DISPLAY_SCALE = 4;

constexpr int wrapBorderSize = 10;

enum class FishingState
{
	STATE_APPEAR = 0,
	STATE_FISHING,
	STATE_REEL,
	STATE_CATCHING,
	STATE_CAUGHT,
	STATE_LOST,
};

struct GameState 
{
	int score = 0;
	float timer = 0;
	int spriteId = 0;
	int winPoints = 0;
	int losePoints = 0;
	FishingState fishingState = FishingState::STATE_APPEAR;
};

GameState gameState;

enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_BASS,
	TYPE_DAB,
	TYPE_ROD,
	TYPE_BAR_UI,
	TYPE_FILL_UI,
	TYPE_FISH_UI,
	TYPE_LETTER_UI,
	TYPE_PROGRESS_UI,
};

// function prototypes
void ScreenWrap(GameObject& obj, Vector2f origin);
void CreateFish(GameObjectType TYPE, int count, const char* sprite_left, const char* sprite_right);
void UpdateFish(GameObjectType TYPE);
void UpdateFishingState();
void SpawnRod();
void UpdateRod();
void PlayerControls();
void SpawnFishingUI();
void UpdateFishingUI();
void PlayerControlsBarUI();
void UpdateFishUI();
void WinFish();
void LoseFish();
void SpawnProgressBarUI();
void UpdateProgressBarUI();
void UpdateScore();

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\underwater.png");
	Play::CentreAllSpriteOrigins();
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	gameState.timer += elapsedTime;
	Play::DrawBackground();
	UpdateScore();
	UpdateFishingState();

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

void CreateFish(GameObjectType TYPE, int count, const char* sprite_left, const char * sprite_right)
{
	for (int count = 0; count < 5; count++) 
	{
		int myFishId = Play::CreateGameObject(TYPE, { rand() % DISPLAY_WIDTH, rand() % DISPLAY_HEIGHT }, 5, sprite_left);
		GameObject& obj_fish = Play::GetGameObject(myFishId);
		
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
		obj_fish.velocity.y = sineMovement * sin(gameState.timer * sineSpeed);

		Play::UpdateGameObject(obj_fish);

		if (Play::IsColliding(obj_fish, obj_rod))
		{
			
			obj_fish.pos = obj_rod.pos;
			obj_rod.velocity.y = -0.5;
			gameState.fishingState = FishingState::STATE_REEL;

			if (obj_rod.pos.y <= -10)
			{
				obj_rod.velocity.y = 0;
				gameState.fishingState = FishingState::STATE_CATCHING;
				Play::DestroyGameObject(id_fish);
			}
		}
	}
}

void UpdateScore()
{
	UpdateFish(TYPE_BASS);
	// bool false 
	// if score more than 2 && bool false
	// UpdateFish(TYPE_DAB);
	// bool == true
}

void PlayerControls() 
{
	GameObject& obj_rod = Play::GetGameObjectByType(TYPE_ROD);

	if (gameState.fishingState == FishingState::STATE_FISHING)
	{
		// rod movement
		if (Play::KeyDown(VK_UP))
		{
			obj_rod.velocity.y = -1;
		}

		else if (Play::KeyDown(VK_DOWN))
		{
			obj_rod.velocity.y = 2;
		}
		// stop rod
		else
		{
			// down
			if (obj_rod.pos.y >= 175)
			{
				obj_rod.velocity.y = 0;
			}
			// up
			else if (obj_rod.pos.y <= -10)
			{
				obj_rod.velocity.y = 0;
			}
		}
	}
}
void SpawnRod() 
{
	Play::CreateGameObject(TYPE_ROD, {150, 10}, 5, "hook");
}

void UpdateRod() 
{
	GameObject& obj_rod = Play::GetGameObjectByType(TYPE_ROD);
	Play::DrawObject(obj_rod);

	Play::DrawLine({ obj_rod.pos.x, 0 }, obj_rod.pos, Play::cWhite);
	Play::UpdateGameObject(obj_rod);
}

void SpawnProgressBarUI() 
{
	Play::CreateGameObject(TYPE_PROGRESS_UI, { 20, 45 }, 10, "progress_bar");
}

void UpdateProgressBarUI() 
{
	GameObject& obj_progress_bar = Play::GetGameObjectByType(TYPE_PROGRESS_UI);
	Play::DrawObject(obj_progress_bar);
	
	Play::UpdateGameObject(obj_progress_bar);
}

void SpawnFishingUI()
{
	Play::CreateGameObject(TYPE_BAR_UI, { 80, 90 }, 10, "bar");
	Play::CreateGameObject(TYPE_FILL_UI, { 80, 90 }, 5, "fill");
	Play::CreateGameObject(TYPE_FISH_UI, { 80, 90 }, 2, "fish_ui");

	GameObject& obj_fish_ui = Play::GetGameObjectByType(TYPE_FISH_UI);
}

void UpdateFishingUI() 
{
	GameObject& obj_bar = Play::GetGameObjectByType(TYPE_BAR_UI);
	GameObject& obj_fill = Play::GetGameObjectByType(TYPE_FILL_UI);
	GameObject& obj_fish_ui = Play::GetGameObjectByType(TYPE_FISH_UI);

	Play::DrawObject(obj_bar);
	Play::DrawObject(obj_fill);
	Play::DrawObject(obj_fish_ui);

	if (Play::IsColliding(obj_fish_ui, obj_fill)) 
	{
		gameState.winPoints++;
	}
	else if (!Play::IsColliding(obj_fish_ui, obj_fill))
	{
		gameState.losePoints++;
	}

	if (gameState.winPoints >= 100) 
	{
		gameState.fishingState = FishingState::STATE_CAUGHT;

	}
	if (gameState.losePoints >= 200) 
	{
		gameState.fishingState = FishingState::STATE_LOST;
	}

	Play::UpdateGameObject(obj_bar);
	Play::UpdateGameObject(obj_fill);
	Play::UpdateGameObject(obj_fish_ui);
}

void UpdateFishUI() 
{
	// random target pos
	// once reach set new random target

	GameObject& obj_fish_ui = Play::GetGameObjectByType(TYPE_FISH_UI);
	// 0 to 1
	float t = (float)(rand() % 1000) / 1000.0f;
	// lerp between random point 0 -> 1
	// 0t is 58, 1t is 122
	int targetPos = 58 + t * (122 - 58);
	int oldPos = obj_fish_ui.pos.y;
	// subtract current pos from target & add fraction of difference to current pos
	// bigger divisor slower movement
	obj_fish_ui.pos.y += (targetPos - oldPos) / 20;

}

void PlayerControlsBarUI() 
{
	GameObject& obj_fill = Play::GetGameObjectByType(TYPE_FILL_UI);

	int barHeightUp = 63;
	int barHeightDown = 117;

	if (Play::KeyPressed(VK_UP))
	{
		obj_fill.velocity.y = -0.5;
	}

	else if (Play::KeyPressed(VK_DOWN))
	{
		obj_fill.velocity.y = 0.5;
	}
	else
	{
		// down
		if (obj_fill.pos.y >= barHeightDown)
		{
			obj_fill.pos.y = barHeightDown;
		}
		// up
		else if (obj_fill.pos.y <= barHeightUp)
		{
			obj_fill.pos.y = barHeightUp;
		}
	}
}

void WinFish()
{
	gameState.score +2;	
	Play::CreateGameObject(TYPE_LETTER_UI, { 160, 90 }, 5, "letter_catch");
	GameObject& obj_letter = Play::GetGameObjectByType(TYPE_LETTER_UI);
	Play::DrawObject(obj_letter);
	Play::UpdateGameObject(obj_letter);

	if (Play::KeyPressed(VK_SPACE)) 
	{
		Play::DestroyGameObjectsByType(TYPE_LETTER_UI);
		gameState.fishingState = FishingState::STATE_FISHING;
	}

}

void LoseFish() 
{
	gameState.fishingState = FishingState::STATE_FISHING;
}

void UpdateFishingState() 
{
	GameObject& obj_rod = Play::GetGameObjectByType(TYPE_ROD);

	switch (gameState.fishingState) 
	{
	case FishingState::STATE_APPEAR: 
		CreateFish(TYPE_BASS, 5, "atlantic_bass", "atlantic_bass_right");
		SpawnRod();
		SpawnProgressBarUI();
		gameState.fishingState = FishingState::STATE_FISHING;
	break;
	case FishingState::STATE_FISHING: 
		gameState.winPoints = 0;
		gameState.losePoints = 0;
		UpdateRod();
		UpdateProgressBarUI();
		PlayerControls();
	break;
	case FishingState::STATE_REEL:
		UpdateRod();
		UpdateProgressBarUI();
	break;
	case FishingState::STATE_CATCHING: 
		SpawnFishingUI();
		UpdateFishingUI();
		UpdateFishUI();
		UpdateProgressBarUI();
		PlayerControlsBarUI();
	break;
	case FishingState::STATE_CAUGHT: 
		Play::DestroyGameObjectsByType(TYPE_BAR_UI);
		Play::DestroyGameObjectsByType(TYPE_FILL_UI);
		Play::DestroyGameObjectsByType(TYPE_FISH_UI);
		UpdateProgressBarUI();
		WinFish();
	break;
	case FishingState::STATE_LOST:
		Play::DestroyGameObjectsByType(TYPE_BAR_UI);
		Play::DestroyGameObjectsByType(TYPE_FILL_UI);
		Play::DestroyGameObjectsByType(TYPE_FISH_UI);
		UpdateProgressBarUI();
		LoseFish();
	break;
	} // end of switch
}
// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

