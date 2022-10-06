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
void PlayerControlsFishUI();
void UpdateFillUI();

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	SpawnRod();
	Play::CentreAllSpriteOrigins();
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	gameState.timer += elapsedTime;
	Play::ClearDrawingBuffer(Play::cCyan);
	UpdateFish();
	UpdateFishingState();
	UpdateFishingUI();
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
		int myFishId = Play::CreateGameObject(TYPE_FISH, { rand() % DISPLAY_WIDTH, rand() % DISPLAY_HEIGHT }, 10, "atlantic_bass");
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
	GameObject& obj_rod = Play::GetGameObjectByType(TYPE_ROD);

	// sine wave variables
	float sineSpeed = 1.5f;
	float sineMovement = 0.2f;
	bool canFish = true;

	for (int id_fish : vFishIds)
	{
		GameObject& obj_fish = Play::GetGameObject(id_fish);

		Vector2f origin = PlayGraphics::Instance().GetSpriteOrigin(obj_fish.spriteId);
		ScreenWrap(obj_fish, origin);

		Play::DrawObject(obj_fish);

		// sine wave
		obj_fish.velocity.y = sineMovement * sin(gameState.timer * sineSpeed);

		Play::UpdateGameObject(obj_fish);

		if (canFish == true) 
		{
			if (Play::IsColliding(obj_fish, obj_rod))
			{
				canFish = false;
				obj_fish.pos = obj_rod.pos;
				obj_rod.velocity.y = -0.5;

				if (obj_rod.pos.y <= -10)
				{
					obj_rod.velocity.y = 0;
					gameState.fishingState = FishingState::STATE_CATCHING;
				}
			}
		}
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
	Play::CreateGameObject(TYPE_ROD, {150, 90}, 5, "hook");
}

void UpdateRod() 
{
	GameObject& obj_rod = Play::GetGameObjectByType(TYPE_ROD);
	Play::DrawObject(obj_rod);

	Play::DrawLine({ obj_rod.pos.x, 0 }, obj_rod.pos, Play::cWhite);
	Play::UpdateGameObject(obj_rod);
}

void SpawnFishingUI()
{
	Play::CreateGameObject(TYPE_BAR_UI, { 80, 90 }, 10, "bar");
	Play::CreateGameObject(TYPE_FILL_UI, { 80, 90 }, 10, "fill");
	Play::CreateGameObject(TYPE_FISH_UI, { 80, 90 }, 10, "fish_ui");
}

void UpdateFishingUI() 
{
	GameObject& obj_bar = Play::GetGameObjectByType(TYPE_BAR_UI);
	GameObject& obj_fill = Play::GetGameObjectByType(TYPE_FILL_UI);
	GameObject& obj_fish_ui = Play::GetGameObjectByType(TYPE_FISH_UI);

	Play::DrawObject(obj_bar);
	Play::DrawObject(obj_fill);
	Play::DrawObject(obj_fish_ui);
	// to win reach x amount of time fish sprite is colliding with fill sprite
	// timer for the mini game runs out and win/lose

	Play::UpdateGameObject(obj_bar);
	Play::UpdateGameObject(obj_fill);
	Play::UpdateGameObject(obj_fish_ui);
}

void UpdateFillUI() 
{
	GameObject& obj_fill = Play::GetGameObjectByType(TYPE_FILL_UI);
	bool canLerp = true;
	// random target pos
	// once reach set new random target

	// 0 to 1
	float t = (float)(rand() % 1000) / 1000.0f;
	// lerp between random point 0 -> 1
	// 0t is 58, 1t is 122
	int targetPos = 58 + t * (122 - 58);
	int oldPos = obj_fill.pos.y;
	// subtract current pos from target & add fraction of  difference to current pos
	// bigger divisor slower movement
	obj_fill.pos.y += (targetPos - oldPos) / 20;

}

void PlayerControlsFishUI() 
{
	GameObject& obj_fish_ui = Play::GetGameObjectByType(TYPE_FISH_UI);

	if (Play::KeyPressed(VK_UP))
	{
		obj_fish_ui.velocity.y = -0.5;
	}

	else if (Play::KeyDown(VK_DOWN))
	{
		obj_fish_ui.velocity.y = 0.5;
	}
	else
	{
		// down
		if (obj_fish_ui.pos.y >= 122)
		{
			obj_fish_ui.pos.y = 122;
		}
		// up
		else if (obj_fish_ui.pos.y <= 58)
		{
			obj_fish_ui.pos.y = 58;
		}
	}

}
void UpdateFishingState() 
{
	GameObject& obj_rod = Play::GetGameObjectByType(TYPE_ROD);

	switch (gameState.fishingState) 
	{
	case FishingState::STATE_APPEAR: 
		SpawnFish();
		SpawnRod();
		gameState.fishingState = FishingState::STATE_FISHING;
	break;
	case FishingState::STATE_FISHING: 
		UpdateRod();
		PlayerControls();
	break;
	case FishingState::STATE_CATCHING: 
		SpawnFishingUI();
		UpdateFishingUI();
		UpdateFillUI();
		PlayerControlsFishUI();
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

