#include "ClearScene.h"
#include "GamePlayingScene.h"

USING_NS_CC;

Scene* Clear::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();
	
	// 'layer' is an autorelease object
	auto layer = Clear::create();
	// add layer as a child to scene
	scene->addChild(layer,0);

	// return the scene
	return scene;
}

bool Clear::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(Clear::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

	// create menu, it's an autorelease object
	auto menu = Menu::create(closeItem, NULL);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu, 1);

	/////////////////////////////
	// 3. add your codes below...

	// add a label shows "Hello World"
	// create and initialize a label
    
	auto label = LabelTTF::create("", "Arial", 24);
    
	// position the label on the center of the screen
	label->setPosition(Vec2(origin.x + visibleSize.width/2,
				origin.y + visibleSize.height - label->getContentSize().height));

	// add the label as a child to this layer
	this->addChild(label, 1);

	// add "GamePlaying" splash screen"
	auto sprite = Sprite::create("clear.png");

	// position the sprite on the center of the screen
	sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

	// add the sprite as a child to this layer
	this->addChild(sprite, 0);

	//_camera = new ActionCamera;
	//_camera->startWithTarget(menu);
	
	auto dispacher = Director::getInstance()->getEventDispatcher();
	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = CC_CALLBACK_2(Clear::onTouchBegan,this);
	listener->onTouchMoved = CC_CALLBACK_2(Clear::onTouchMoved,this);
	dispacher->addEventListenerWithSceneGraphPriority(listener,this);
	Director::getInstance()->setProjection(Director::Projection::_2D);
	scheduleUpdate();
	
	runAction(
		Repeat::create(
		
			Sequence::create(
				Place::create(Vec2(0,-8)),
				DelayTime::create(0.1f),
				Place::create(Vec2(0,0)),
				DelayTime::create(0.1f),
				nullptr),
				3)
				);

	return true;
}



void 
Clear::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags){
	//Director::getInstance()->setProjection(Director::Projection::_2D);
}

void Clear::update(float delta){
	//Director::getInstance()->setProjection(Director::Projection::_2D);
	//Director::getInstance()->loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
	//Mat4 orthoMatrix;
	//Mat4 m = Mat4::IDENTITY;
	//auto size = Director::getInstance()->getWinSize();
 //       Mat4::createOrthographicOffCenter(0, size.width, 0, size.height, -1024, 1024, &orthoMatrix);
	//setAdditionalTransform(&orthoMatrix);
}

bool Clear::onTouchBegan(cocos2d::Touch* , cocos2d::Event*){
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f,GamePlaying::createScene()));
	return true;
}
	
void Clear::onTouchMoved(cocos2d::Touch* t,cocos2d::Event*){
	//kmVec3 eye = _camera->getEye();
	//eye.x -= t->getDelta().x * 0.000000001;
	//eye.y -= t->getDelta().y * 0.000000001;
	//_camera->setEye(eye);
}

void Clear::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
