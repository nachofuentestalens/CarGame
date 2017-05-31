//Copyright (C) 2016 Nacho Fuentes Talens

#ifndef __MAIN_MENU_SCENE_H__
#define __MAIN_MENU_SCENE_H__

#include "cocos2d.h"
#include "ui\UIEditBox\UIEditBox.h"
//#include "cocos-ext.h"
#include <thread>
#include <vector>

using namespace cocos2d;
//using namespace cocos2d::extension;
using namespace std;

class MainMenu : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
	bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event);

	bool RegisterUser(string name, string password);
	bool LoginUser(string name, string password);
	static void UpdateFeedbackLabel(string msg);
	void Update(float dt);
	float kUpdateTick = 1.0;

    // a selector callback
    void menuCloseCallback(Ref* pSender);
	ui::EditBox* userBox;
	ui::EditBox* passBox;
	Size winSize;
	Label* labelLogin;
	Label* labelRegister;
	static Label* labelFeedback;
	static bool bShouldStartGame;
    // implement the "static create()" method manually
    CREATE_FUNC(MainMenu);
};

#endif // __MAIN_MENU_SCENE_H__

