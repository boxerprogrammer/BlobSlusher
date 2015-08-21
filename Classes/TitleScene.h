#pragma once 
#include "cocos2d.h"

class Title : public cocos2d::Layer
{
	private:

		cocos2d::ActionCamera* _camera;
	public:
		// there's no 'id' in cpp, so we recommend returning the class instance pointer
		static cocos2d::Scene* createScene();

		// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
		virtual bool init();  
    
		// a selector callback
		void menuCloseCallback(cocos2d::Ref* pSender);
    
		bool onTouchBegan(cocos2d::Touch* , cocos2d::Event*);
		void onTouchMoved(cocos2d::Touch*,cocos2d::Event*);
		
		void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags);
		void update(float delta);

		// implement the "static create()" method manually
		CREATE_FUNC(Title);
};

