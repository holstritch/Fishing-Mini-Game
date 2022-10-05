#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 320;
int DISPLAY_HEIGHT = 180;
int DISPLAY_SCALE = 4;

constexpr int wrapBorderSize = 10.0f;

enum class FishingState
{
	STATE_APPEAR = 0,
	STATE_FISHING,
	STATE_CATCHING,
	STATE_CAUGHT,
};

struct GameState 
{
	int score = 0;
	float timer = 0;
	int spriteId = 0;
	FishingState fishingState = FishingState::STATE_APPEAR;
};

GameState gameState;

enum GameObjectType
{
	TYPE_NULL = -1,
	TYPE_FISH,
	TYPE_ROD,
	TYPE_BAR_UI,
	TYPE_FILL_UI,
	TYPE_FISH_UI,
};

// function prototypes
void ScreenWrap(GameObject& obj, Vector2f origin);
void SpawnFish();
void UpdateFish();
void UpdateFishingState();
void SpawnRod();
void UpdateRod();
void PlayerControls();
void SpawnFishingUI();
void UpdateFishingUI();

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	SpawnRod();
	SpawnFishingUI();
	Play::CentreAllSpriteOrigins();
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	gameState.timer += elapsedTime;
	Play::ClearDrawingBuffer(Play::cCyan);
	UpdateFish();
	UpdateFishingState();
	UpdateRod();
	UpdateFishingUI();
	PlayerControls();
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

void SpawnFish() 
{
	for (int i = 0; i < 5; i++) 
	{
		int myFishId = Play::CreateGameObject(TYPE_FISH, { rand() % DISPLAY_WIDTH, rand() % DISPLAY_HEIGHT }, 50, "atlantic_bass");
		GameObject& obj_fish = Play::GetGameObject(myFishId);
		
		// (0 -> 999) / 1000.0
		float randomRatio = (rand() % 1000) / 1000.0f;

		obj_fish.velocity.x = 0.3f + (randomRatio * 0.5f);

		// coin flip left or right 
		if ((rand() % 100) < 50) obj_fish.velocity.x = -obj_fish.velocity.x;

	}
}
void UpdateFish() 
{
	std::vector<int> vFishIds = Play::CollectGameObjectIDsByType(TYPE_FISH);

	// sine wave variables
	float sineSpeed = 1.5f;
	float sineMovement = 0.2f;

	for (int id_fish : vFishIds)
	{
		GameObject& obj_fish = Play::GetGameObject(id_fish);

		Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_fish.spriteId);
		ScreenWrap(obj_fish, origin);

		Play::DrawObject(obj_fish);

		// sine wave
		obj_fish.velocity.y = sineMovement * sin(gameState.timer * sineSpeed);

		Play::UpdateGameObject(obj_fish);
	}
}

void PlayerControls() 
{
	GameObject& obj_rod = Play::GetGameObjectByType(TYPE_ROD);

	if (gameState.fishingState == FishingState::STATE_FISHING)
	{
		// rod movement
		if (Play::KeyDown(VK_UP))
		{
			obj_rod.velocity.y = -0.5;
		}

		else if (Play::KeyDown(VK_DOWN))
		{
			obj_rod.velocity.y = 1;
		}
		// stop rod
		else
		{
			if (obj_rod.pos.y >= 175)
			{
				obj_rod.velocity.y = 0;
			}

			else if (obj_rod.pos.y <= -10)
			{
				obj_rod.velocity.y = 0;
			}
		}
	}
}
void SpawnRod() 
{
	Play::CreateGameObject(TYPE_ROD, {150, 90}, 20, "hook");
}

void UpdateRod() 
{
	GameObject& obj_rod = Play::GetGameObjectByType(TYPE_ROD);
	Play::DrawObject(obj_rod);

	PlayerControls();
	Play::DrawLine({ obj_rod.pos.x, 0 }, obj_rod.pos, Play::cWhite);
	Play::UpdateGameObject(obj_rod);
}

void SpawnFishingUI()
{
	Play::CreateGameObject(TYPE_BAR_UI, { 37, 45 }, 20, "bar");
	Play::CreateGameObject(TYPE_FILL_UI, { 37, 45 }, 20, "fill");
	Play::CreateGameObject(TYPE_FISH_UI, { 37, 45 }, 20, "fish_ui");
}

void UpdateFishingUI() 
{
	GameObject& obj_bar = Play::GetGameObjectByType(TYPE_BAR_UI);
	GameObject& obj_fill = Play::GetGameObjectByType(TYPE_FILL_UI);
	GameObject& obj_fish_ui = Play::GetGameObjectByType(TYPE_FISH_UI);

	Play::DrawObject(obj_bar);
	Play::DrawObject(obj_fill);
	Play::DrawObject(obj_fish_ui);

	Play::UpdateGameObject(obj_bar);
	Play::UpdateGameObject(obj_fill);
	Play::UpdateGameObject(obj_fish_ui);
}
void UpdateFishingState() 
{
	switch (gameState.fishingState) 
	{
	case FishingState::STATE_APPEAR: 
		SpawnFish();
		SpawnRod();
		gameState.fishingState = FishingState::STATE_FISHING;
	break;
	case FishingState::STATE_FISHING: 
		UpdateRod();
	break;
	case FishingState::STATE_CATCHING: 
		
	break;
	case FishingState::STATE_CAUGHT: 
		
	break;
	} // end of switch
}
// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

