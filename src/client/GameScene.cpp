//Copyright (C) 2016 Nacho Fuentes Talens

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "stdio.h"
#include "winsock2.h"
#include <ws2tcpip.h>
#include <time.h>
#include <iostream>
#include <mutex>
#include <string>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")
#include "GameScene.h"
#include "Networking.h"

USING_NS_CC;

//winsock data
//WSADATA wsa;
//SOCKET udp_socket;
//sockaddr_in ip;
//sockaddr_in ips;

struct position {
	int X;
	int Y;
};

struct client {
	int id;
	sockaddr_in ip;
	position pos;
	std::string name;
	std::string password;
	int lives;
};

Scene* GameScene::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = GameScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
bool GameScene::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	winSize = Director::getInstance()->getWinSize();

	//Enable Keypad
	this->setKeypadEnabled(true);

	/////////////////////////////
	// 3. add your codes below...
	// add bg
	auto sprite = Sprite::create("AsphaltMap.png");
	sprite->setScaleX(winSize.width / sprite->getContentSize().width);
	sprite->setScaleY(winSize.height / sprite->getContentSize().height);
	sprite->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));
	this->addChild(sprite, 0);

	owning_player = Sprite::create("car_black_4.png");
	switch (Networking::Instance().owning_player.team) {
	case 1: owning_player->setTexture(CCTextureCache::sharedTextureCache()->addImage("car_green_4.png"));
		break;
	case 2: owning_player->setTexture(CCTextureCache::sharedTextureCache()->addImage("car_blue_4.png"));
		break;
	case 3: owning_player->setTexture(CCTextureCache::sharedTextureCache()->addImage("car_yellow_4.png"));
		break;
	}
	owning_player->setPosition(Networking::Instance().owning_player.pos.X, Networking::Instance().owning_player.pos.Y);
	owning_player->setScale(0.3f);
	this->addChild(owning_player, 10);
	owning_name = Label::createWithTTF(Networking::Instance().owning_player.name, "fonts/Marker Felt.ttf", 20);
	owning_name->setPosition(owning_player->getPosition().x, owning_player->getPosition().y + 10);
	this->addChild(owning_name, 10);
	owning_name->setColor(Color3B::BLACK);

	string s;
	ostringstream convert;
	convert << Networking::Instance().owning_player.lives;
	s = convert.str();
	owning_lives = Label::createWithTTF(s, "fonts/Marker Felt.ttf", 20);
	owning_lives->setPosition(owning_player->getPosition().x, owning_player->getPosition().y - 15);
	this->addChild(owning_lives, 10);
	owning_lives->setColor(Color3B::BLACK);

	for (unsigned int i = 0; i < kMaxPlayers; ++i) {
		Sprite* tmp = Sprite::create("car_black_5.png");
		tmp->setPosition(-200.0f, -200.0f);
		tmp->setScale(0.3f);
		tmp->setOpacity(0);
		playerSprites.push_back(tmp);
		this->addChild(tmp, 4);

		Label* lab = Label::createWithTTF("", "fonts/Marker Felt.ttf", 20);
		lab->setColor(Color3B::BLACK);
		this->addChild(lab, 4);
		playerNames.push_back(lab);

		Label* lab2 = Label::createWithTTF("", "fonts/Marker Felt.ttf", 20);
		lab2->setColor(Color3B::BLACK);
		this->addChild(lab2, 4);
		playerLives.push_back(lab2);
	}

	this->schedule(schedule_selector(GameScene::Update), kUpdateTick);

	return true;
}

bool GameScene::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) {

	return true;
}

void GameScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* unused_event) {
	if (Networking::Instance().owning_player.lives > 0) {
		switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_W:
			pressedW = true;
			break;
		case EventKeyboard::KeyCode::KEY_A:
			pressedA = true;
			break;
		case EventKeyboard::KeyCode::KEY_S:
			pressedS = true;
			break;
		case EventKeyboard::KeyCode::KEY_D:
			pressedD = true;
			break;
		}
	}
}

void GameScene::Update(float dt) {

	if (pressedW) {
		if (owning_player->getPosition().y < 440 + player_speed) owning_player->setPosition(owning_player->getPosition().x, owning_player->getPosition().y + player_speed);
		owning_player->setRotation(0.0f);
	}
	if (pressedS) {
		if (owning_player->getPosition().y > 50 - player_speed) owning_player->setPosition(owning_player->getPosition().x, owning_player->getPosition().y - player_speed);
		owning_player->setRotation(180.0f);
	}
	if (pressedA) {
		if (owning_player->getPosition().x > 70 - player_speed) owning_player->setPosition(owning_player->getPosition().x - player_speed, owning_player->getPosition().y);
		owning_player->setRotation(-90.0f);
	}
	if (pressedD) {
		if (owning_player->getPosition().x < 870 + player_speed) owning_player->setPosition(owning_player->getPosition().x + player_speed, owning_player->getPosition().y);
		owning_player->setRotation(90.0f);
	}
	owning_name->setPosition(owning_player->getPosition().x, owning_player->getPosition().y + 30);
	string s;
	ostringstream convert;
	convert << Networking::Instance().owning_player.lives;
	s = convert.str();
	owning_lives->setString(s);
	owning_lives->setPosition(owning_player->getPosition().x, owning_player->getPosition().y - 30);

	UpdateData();
	UpdatePlayerSprites();

	if (RecoveringFromFriendCrash > 0.0f) {
		RecoveringFromFriendCrash -= dt;
		owning_lives->setColor(Color3B::RED);
	}
	else owning_lives->setColor(Color3B::BLACK);
	if (RecoveringFromEnemyCrash > 0.0f) {
		RecoveringFromEnemyCrash -= dt;
		redColor *= -1;
		playerRed -= redColor;
		owning_player->setOpacity(playerRed);
	}
	else owning_player->setOpacity(255);

	if (Networking::Instance().owning_player.lives <= 0) {
		owning_player->setOpacity(0.0f);
		owning_lives->setOpacity(0.0f);
		owning_name->setOpacity(0.0f);
	}

}

void GameScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* unused_event) {
	switch (keyCode) {
	case EventKeyboard::KeyCode::KEY_W:
		pressedW = false;
		break;
	case EventKeyboard::KeyCode::KEY_A:
		pressedA = false;
		break;
	case EventKeyboard::KeyCode::KEY_S:
		pressedS = false;
		break;
	case EventKeyboard::KeyCode::KEY_D:
		pressedD = false;
		break;
	}
}

void GameScene::UpdatePlayerSprites() {
	Rect owningCollision = owning_player->getBoundingBox();
	Rect otherPlayer;
	for (unsigned int i = 0; i < Networking::Instance().players.size(); ++i) {
		if (Networking::Instance().players[i].lives <= 0) {
			playerSprites[i]->setOpacity(0.0f);
			playerLives[i]->setOpacity(0.0f);
			playerNames[i]->setOpacity(0.0f);
		}
		else {
			/*playerSprites[i]->setOpacity(255.0f);
			playerLives[i]->setOpacity(255.0f);*/
			playerSprites[i]->setPosition(Networking::Instance().players[i].pos.X, Networking::Instance().players[i].pos.Y);
			playerSprites[i]->setRotation(Networking::Instance().players[i].rotation);
			playerNames[i]->setString(Networking::Instance().players[i].name);
			playerNames[i]->setPosition(Networking::Instance().players[i].pos.X, Networking::Instance().players[i].pos.Y + 30);
			string s;
			ostringstream convert;
			convert << Networking::Instance().players[i].lives;
			s = convert.str();
			playerLives[i]->setString(s);
			playerLives[i]->setPosition(Networking::Instance().players[i].pos.X, Networking::Instance().players[i].pos.Y - 30);
			/*if (Networking::Instance().players[i].team == Networking::Instance().owning_player.team) {
				playerSprites[i]->setTexture(CCTextureCache::sharedTextureCache()->addImage("car_green_5.png"));
				playerSprites[i]->setOpacity(255.0f);
			}
			else {
				playerSprites[i]->setTexture(CCTextureCache::sharedTextureCache()->addImage("car_red_5.png"));
				playerSprites[i]->setOpacity(255.0f);
			}*/
			switch (Networking::Instance().players[i].team) {
			case 1: playerSprites[i]->setTexture(CCTextureCache::sharedTextureCache()->addImage("car_green_5.png"));
				playerSprites[i]->setOpacity(255.0f);
				break;
			case 2: playerSprites[i]->setTexture(CCTextureCache::sharedTextureCache()->addImage("car_blue_5.png"));
				playerSprites[i]->setOpacity(255.0f);
				break;
			case 3: playerSprites[i]->setTexture(CCTextureCache::sharedTextureCache()->addImage("car_yellow_5.png"));
				playerSprites[i]->setOpacity(255.0f);
				break;
			}
			otherPlayer = playerSprites[i]->getBoundingBox();
			if (owningCollision.intersectsRect(otherPlayer)) {
				if (Networking::Instance().players[i].team == Networking::Instance().owning_player.team && RecoveringFromFriendCrash <= 0.0f) {
					if (Networking::Instance().players[i].lives > 0 && Networking::Instance().owning_player.lives > 0) {
						Networking::Instance().owning_player.lives++;
						RecoveringFromFriendCrash = kRecoverTimeFriendly;
					}
				}
				if (Networking::Instance().players[i].team != Networking::Instance().owning_player.team && RecoveringFromEnemyCrash <= 0.0f) {
					if (Networking::Instance().players[i].lives > 0 && Networking::Instance().owning_player.lives > 0) {
						Networking::Instance().owning_player.lives--;
						RecoveringFromEnemyCrash = kRecoverTimeEnemy;
					}
				}
			}
		}
	}
}

void GameScene::UpdateData() {
	Networking::Instance().owning_player.pos.X = owning_player->getPosition().x;
	Networking::Instance().owning_player.pos.Y = owning_player->getPosition().y;
	Networking::Instance().owning_player.rotation = owning_player->getRotation();
	Networking::Instance().UpdateOwnerData();
}
//void Game::menuCloseCallback(Ref* pSender)
//{
//    Director::getInstance()->end();
//	for (unsigned int i = 0; i < threads.size(); ++i) {
//		threads[i].detach();
//	}
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
//    exit(0);
//#endif
//}
