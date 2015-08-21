#include "AppDelegate.h"
#include "TitleScene.h"

USING_NS_CC;

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
}

#ifdef _WINDOWS
	const float WIDTH=420;
	const float HEIGHT=700;
#else
	const int WIDTH=480;
	const int HEIGHT=800;
#endif


bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
#ifdef _WINDOWS
        glview = GLViewImpl::createWithRect("EvilBoard",Rect(0,0,480,800),WIDTH/480.0f);
#else
		glview = GLView::create("SuperSlushman");
#endif
        director->setOpenGLView(glview);
		glview->setFrameSize(480,800);

	
    }

    // turn on display FPS
    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0 / 60);

	glview->setDesignResolutionSize(480,800,ResolutionPolicy::SHOW_ALL);

    // create a scene. it's an autorelease object
    auto scene = Title::createScene();

    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
