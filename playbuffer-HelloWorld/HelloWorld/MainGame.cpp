#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 320;
int DISPLAY_HEIGHT = 180;
int DISPLAY_SCALE = 4;

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
};

// function prototypes
void SpawnFish();
void UpdateFish();
void UpdateFishingState();
void SpawnRod();
void UpdateRod();

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
    Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
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
	Play::PresentDrawingBuffer();

	return Play::KeyDown(VK_ESCAPE);
}

void SpawnFish() 
{
	for (int i = 0; i < 5; i++) 
	{
		int myFishId = Play::CreateGameObject(TYPE_FISH, { rand() % DISPLAY_WIDTH, rand() % DISPLAY_HEIGHT }, 50, "atlantic_bass");
		GameObject& obj_Fish = Play::GetGameObject(myFishId);

		// rand velocity x axis left or right 
	}
}
void UpdateFish() 
{
	std::vector<int> vFishIds = Play::CollectGameObjectIDsByType(TYPE_FISH);

	for (int id_fish : vFishIds)
	{
		GameObject& obj_fish = Play::GetGameObject(id_fish);

		Play::DrawObject(obj_fish);

		Play::UpdateGameObject(obj_fish);

	}
}

void SpawnRod() 
{

}

void UpdateRod() 
{
	// rod movement
}
void UpdateFishingState() 
{
	switch (gameState.fishingState) 
	{
	case FishingState::STATE_APPEAR: 
		SpawnFish();
		SpawnRod();
		gameState.fishingState = FishingState::STATE_CATCHING;
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

