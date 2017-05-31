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
#include "MainMenuScene.h"
#include "GameScene.h"	
#include "Networking.h"

USING_NS_CC;

Label* MainMenu::labelFeedback;
bool MainMenu::bShouldStartGame;
//winsock data
WSADATA wsa;
SOCKET udp_socket;
sockaddr_in ip;
sockaddr_in ips;

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

void ReceiveData() {
	char* ptr;
	int size = sizeof(ip);
	char buffer[250];
	char answer[250];
	char cID[10];
	int bytes = 0;
	memset(buffer, 0, 250);
	memset(answer, 0, 250);
	while (1) {
		bytes = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (SOCKADDR*)&ips, &size);
		if (bytes > 0) {
			ptr = strtok(buffer, ":");
			if (strcmp(ptr, "RG_IDENTIFY") == 0) {
				ptr = strtok(buffer, ":");
				UserDefault::getInstance()->setIntegerForKey("Id", atoi(ptr));
			}
			else {
				if (strcmp(ptr, "RG_LOGIN_SUCCESSFUL") == 0) {

				}
				else {
					if (strcmp(ptr, "RG_LOGIN_ERROR") == 0) {

					}
				}
			}
		}
	}
}

Scene* MainMenu::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = MainMenu::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool MainMenu::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
	winSize = Director::getInstance()->getWinSize();

	//Create Touch listener
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(MainMenu::onTouchBegan, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(MainMenu::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...
    // add bg
    auto sprite = Sprite::create("menubg.png");
	sprite->setScaleX(winSize.width / sprite->getContentSize().width);
	sprite->setScaleY(winSize.height / sprite->getContentSize().height);
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
    this->addChild(sprite, 0);

	//char buffer[250];
	//memset(buffer, 0, 250);
	//int bytes = 0;
	////WINSOCK
	//WSAStartup(MAKEWORD(2, 0), &wsa);
	//udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//ip.sin_family = AF_INET;
	//ip.sin_port = htons(8888);
	//ip.sin_addr.s_addr = inet_addr("255.255.255.255");
	//sprintf(buffer, "RG_DISCOVER");
	//int valor = 1;
	//if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (char*)&valor, sizeof(valor))) {
	//	printf("Broadcast socket error");
	//	return 1;
	//}
	//sendto(udp_socket, buffer, sizeof(buffer), 0, (SOCKADDR *)&ip, sizeof(ip));
	//valor = 0;
	//if (setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (char*)&valor, sizeof(valor))) {
	//	printf("undo Broadcast socket error");
	//	return 1;
	//}
	//threads.push_back(std::thread (ReceiveData));

	Size editBoxSize;
	string pNormalSprite = "barbg.png";
	editBoxSize.height = 30.0f;
	editBoxSize.width = 450.0f;
	
	userBox = ui::EditBox::create(editBoxSize, ui::Scale9Sprite::create(pNormalSprite));
	userBox->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height * 0.7));
	userBox->setFontSize(20);
	userBox->setFontColor(Color3B::BLACK);
	userBox->setPlaceHolder("Name:");
	userBox->setPlaceholderFontColor(Color3B::BLACK);
	userBox->setMaxLength(20);
	userBox->setInputMode(ui::EditBox::InputMode::SINGLE_LINE);
	userBox->setReturnType(ui::EditBox::KeyboardReturnType::DONE);
	userBox->setInputFlag(ui::EditBox::InputFlag::INITIAL_CAPS_WORD);
	addChild(userBox);

	passBox = ui::EditBox::create(editBoxSize, ui::Scale9Sprite::create(pNormalSprite));
	passBox->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height * 0.6));
	passBox->setFontSize(20);
	passBox->setFontColor(Color3B::BLACK);
	passBox->setPlaceHolder("Password:");
	passBox->setPlaceholderFontColor(Color3B::BLACK);
	passBox->setMaxLength(20);
	passBox->setInputMode(ui::EditBox::InputMode::SINGLE_LINE);
	passBox->setInputFlag(ui::EditBox::InputFlag::INITIAL_CAPS_WORD);
	passBox->setInputFlag(ui::EditBox::InputFlag::PASSWORD);
	passBox->setReturnType(ui::EditBox::KeyboardReturnType::DONE);
	addChild(passBox);

	labelLogin = Label::createWithTTF("Login", "fonts/Marker Felt.ttf", 30);
	labelLogin->setPosition(Vec2(origin.x + visibleSize.width * 0.3, origin.y + visibleSize.height * 0.5));
	this->addChild(labelLogin, 1);
	labelLogin->setColor(Color3B::WHITE);

	labelRegister = Label::createWithTTF("Register", "fonts/Marker Felt.ttf", 30);
	labelRegister->setPosition(Vec2(origin.x + visibleSize.width * 0.5, origin.y + visibleSize.height * 0.5));
	this->addChild(labelRegister, 1);
	labelRegister->setColor(Color3B::WHITE);

	labelFeedback = Label::createWithTTF("", "fonts/Marker Felt.ttf", 20);
	labelFeedback->setPosition(Vec2(origin.x + visibleSize.width * 0.4, origin.y + visibleSize.height * 0.4));
	this->addChild(labelFeedback, 1);
	labelFeedback->setColor(Color3B::WHITE);

	Networking::Instance();

	this->schedule(schedule_selector(MainMenu::Update), kUpdateTick);

	bShouldStartGame = false;

    return true;
}

void MainMenu::Update(float dt) {
	if (bShouldStartGame) {
		_eventDispatcher->removeAllEventListeners();
		auto gameScene = GameScene::createScene();
		Director::getInstance()->replaceScene(gameScene);
	}
}

bool MainMenu::RegisterUser(string name, string password) {
	ofstream myfile("tmp.txt", ios::out | ios::app);
	myfile << endl << name << "  " << password << endl;
	myfile.close();
	return true;
}

bool MainMenu::LoginUser(string name, string password) {
	string tmp_name, tmp_password;
	ifstream myfile("tmp.txt");
	bool found = false;
	if (myfile.is_open()) {
		while (!myfile.eof()) {
			myfile >> tmp_name >> tmp_password;
			if (name == tmp_name && password == tmp_password)
			{
				found = true;
				break;
			}
		}
		myfile.close();
	}
	if (found) return true;
	return false;
}

bool MainMenu::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event) {
	if (labelLogin->getBoundingBox().containsPoint(touch->getLocation())) {
		//Login
		//LoginUser(userBox->getText(), passBox->getText());
		Networking::Instance().LoginUser(userBox->getText(), passBox->getText(), UserDefault::getInstance()->getIntegerForKey("Id"));
	}
	else {
		if (labelRegister->getBoundingBox().containsPoint(touch->getLocation())) {
			//Register
			//RegisterUser(userBox->getText(), passBox->getText());
			Networking::Instance().RegisterUser(userBox->getText(), passBox->getText());
		}
	}
	return true;
}

void MainMenu::UpdateFeedbackLabel(string msg) {
	labelFeedback->setString(msg);
	if (msg == "Login Successful") {
		bShouldStartGame = true;
	}
}

void MainMenu::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
