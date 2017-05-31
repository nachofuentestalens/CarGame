//Copyright (C) 2016 Nacho Fuentes Talens

#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"

using namespace cocos2d;
using namespace std;

class GameScene : public cocos2d::Layer
{
public:
	struct Bombs : public cocos2d::Sprite {
		int team;
	};
    static cocos2d::Scene* createScene();

    virtual bool init();
    
	bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);
	void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* unused_event);
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* unused_event);
	void Update(float dt);
	void UpdatePlayerSprites();
	void UpdateData();
	const float kUpdateTick = 0.08f;


	bool pressedW;
	bool pressedA;
	bool pressedS;
	bool pressedD;
	const float kRecoverTimeFriendly = 3.0f;
	const float kRecoverTimeEnemy = 1.0f;
	float RecoveringFromFriendCrash = 0.0f;
	float RecoveringFromEnemyCrash = 0.0f;
	float elapsedTime = 0.0f;
	
	Size winSize;
	Label* labelLogin;
	Label* labelRegister;
	Sprite* owning_player;
	Label* owning_name;
	Label* owning_lives;
	Bombs* owning_bomb;
	vector<Sprite*> playerSprites;
	vector<Label*> playerNames;
	vector<Label*> playerLives;
	vector<Bombs*> playerBombs;
	float player_speed = 8.0f;
	const int kMaxPlayers = 20;
	int redColor = 255;
	int playerRed = 0;
    // implement the "static create()" method manually
    CREATE_FUNC(GameScene);
};

#endif // __GAME_SCENE_H__

