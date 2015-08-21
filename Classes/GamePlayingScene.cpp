#include "GamePlayingScene.h"
#include "FailedScene.h"
#include "ClearScene.h"
#include "DrawNodeWithTex.h"
#include"CCGL.h"
#include<chipmunk.h>
//#include<physics\chipmunk\CCPhysicsShapeInfo_chipmunk.h>
USING_NS_CC;

const float BORDER_WIDTH=1.5f;
#ifndef _WINDOWS
	#include<platform\android\jni\JniHelper.h>//日本語変換用
	std::string GetUTF8FromSJIS(const char* sjisstr){
		cocos2d::JniMethodInfo methodInfo;
		if(cocos2d::JniHelper::getStaticMethodInfo(methodInfo,
			"org/cocos2dx/cpp/AppActivity",
			"getUTFStringFromSJIS",
			"([B)[B")){
			jbyteArray instr=methodInfo.env->NewByteArray(strlen(sjisstr)+1);
			jbyte* pstr = methodInfo.env->GetByteArrayElements(instr,0);
			strcpy((char*)pstr,sjisstr);
			jbyteArray str= (jbyteArray)methodInfo.env->CallStaticObjectMethod(methodInfo.classID,methodInfo.methodID,instr);
			const char* nstr = (char*)methodInfo.env->GetByteArrayElements(str,nullptr);
			std::string retval=nstr;
			int len=methodInfo.env->GetArrayLength(str);
			methodInfo.env->ReleaseByteArrayElements(str,(jbyte*)nstr,0);
			return retval;
		}else{
			return std::string("");
		}
	}

	std::string GetTestString(){
		cocos2d::JniMethodInfo methodInfo;
			if(cocos2d::JniHelper::getStaticMethodInfo(methodInfo,
			"org/cocos2dx/cpp/AppActivity",
			"getTestString",
			"()Ljava/lang/String;")){
				return "";
			}
			return "";
	}



#endif



//WINDOWS用SJIS→UTF8
#ifdef _WINDOWS
char*   GetUTF8FromSJIS(const char* src){
	unsigned int sizeWide = MultiByteToWideChar(CP_ACP, 0, src, -1, nullptr, 0);
	wchar_t* bufferWide = new wchar_t[sizeWide+1]();
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, bufferWide, sizeWide);
	unsigned int sizeUtf8 = WideCharToMultiByte(CP_UTF8, 0, bufferWide, -1, nullptr, 0, nullptr, nullptr);
	char* bufferUtf8 = new char[sizeUtf8+1]();
	WideCharToMultiByte(CP_UTF8, 0, bufferWide, -1, bufferUtf8, sizeUtf8, nullptr, nullptr);
	delete bufferWide;
	return bufferUtf8;
}
#endif
///@todo
//画面外ポリゴンの削除
//重心、バウンスの調整
///@memo
//足場ベクタはsetAdditionalTransform
//面積:s=(a+b+c)/2
//面積=sqrt(s(s-a)(s-b)(s-c))

static int stage_no=0;

float cross2d(const cocos2d::Vec2& a, const cocos2d::Vec2& b){
	return a.x*b.y-a.y*b.x;
}

float angle2d(const cocos2d::Vec2& a, const cocos2d::Vec2& b){
	return atan2(cross2d(a,b),Vec2::dot(a,b));
}


cocos2d::Vec2 GetMidUV(const cocos2d::Vec2& startPos,
					const cocos2d::Vec2& startUV,
					const cocos2d::Vec2& endPos,
					const cocos2d::Vec2& endUV,
					const cocos2d::Vec2& midPos){
	if(fabsf(endPos.x-startPos.x)>fabsf(endPos.y-startPos.y)){
		float t=(midPos.x-startPos.x)/(endPos.x-startPos.x);
		return startUV*(1-t)+endUV*t;
	}else{
		float t=(midPos.y-startPos.y)/(endPos.y-startPos.y);
		return startUV*(1-t)+endUV*t;
	}
}

DrawNodeWithTex* CreateCuttablePolygon(cocos2d::ValueMap& map){
	return nullptr;
}



cocos2d::Node* 
GamePlaying::CreateScaffoldRectNode(cocos2d::ValueMap& map){
	return nullptr;
}


///@attention DrawPolygon関数は時計回りしか受け付けない
PossesAndUvs 
GamePlaying::GetPolygonVerticesInfo(cocos2d::ValueMap& map){
	PossesAndUvs posuvs=PossesAndUvs();
	auto v = map.find("points");
	std::vector<Vec2>& posses=posuvs.posses;
	if(v==map.end()){
		return posuvs;
	}else{
		
		for(auto& mm : v->second.asValueVector()){
			//残念ながらTileMapEditorのy軸は上下さかさまである…
			posses.push_back(Vec2(mm.asValueMap()["x"].asInt(),-mm.asValueMap()["y"].asInt()));
		}
	}

	//中心点(平均点)の計算
	Vec2 center=Vec2();
	for(auto& p : posses){
		center+=p;
	}
	//反時計回りなら反転
	center=center/posses.size();
	if((posses[0]-center).cross(posses[1]-center)>0){
		std::reverse(posses.begin(),posses.end());
	}
	//中心合わす
	posuvs.center = center;
	for(auto& vert : posses){
		vert-=center;
	}

	std::vector<Vec2>& uvs=posuvs.uvs;
	uvs.push_back(Vec2(0,0));
	uvs.push_back(Vec2(1,0));
	uvs.push_back(Vec2(1,1));
	uvs.push_back(Vec2(0,1));
	return posuvs;
}


Scene* GamePlaying::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::createWithPhysics();
	scene->getPhysicsWorld()->setAutoStep(true);
	scene->getPhysicsWorld()->setGravity(Vec2(0,-198));
	//scene->getPhysicsWorld()->setDebugDrawMask(0xff);//デバッグ用
    
	// 'layer' is an autorelease object
	auto layer = GamePlaying::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

void
GamePlaying::CheckAndRemovePolygons(){
	//
	for(int t=0;t<=_tag;++t){
		auto node = getChildByTag(t);
		if(node!=nullptr&&node->getPositionY()<-100){
	 		removeChild(node);
		}
	}
}

bool
GamePlaying::CheckRemainAnyPolygons(){
	for(int t=0;t<=_tag;++t){
		auto node = getChildByTag(t);
		if(node!=nullptr){
			return true;
		}
	}
	return false;
}

bool 
GamePlaying::CheckAllPolygonsStopped(){
	for(int t=0;t<=_tag;++t){
		auto node = getChildByTag(t);
		if(node==nullptr){
			continue;
		}
		Vec2 vel=node->getPhysicsBody()->getVelocity(); 
		if(vel.length()>0.001){
			return false;
		}
	}
	return true;
}

void
GamePlaying::update(float delta){
	CheckAndRemovePolygons();
	if(_slushLimit==0 && !_trans){
		if(!CheckRemainAnyPolygons()){
			_trans=true;
			stage_no = (stage_no==3?0:stage_no+1);
			Director::getInstance()->replaceScene(TransitionJumpZoom::create(0.3f,Clear::createScene()));
		}else if(CheckAllPolygonsStopped()){
			//_trans=true;
			//Director::getInstance()->replaceScene(TransitionFadeDown::create(0.3f,Failed::createScene()));
		}
	}
}


void
GamePlaying::ObjectLayerParse(TMXObjectGroup* og,int& priority ){
	if(og==nullptr){
		return;
	}
	auto objs = og->getObjects();

	int assertcount=0;
	for(auto& obj : objs){
		ValueMap& map = obj.asValueMap();
		
		std::string type = map["type"].asString();
		Vec2 pos(map["x"].asFloat(),map["y"].asFloat());
		if(type=="cuttable"){	
			PossesAndUvs pau=GetPolygonVerticesInfo(map);
			if(!pau.posses.empty()){
				///ポリゴンの生成
				MyPolygon p;
				std::vector<Vec2>& v = p.vertices;
				for(auto& pos : pau.posses){
					v.push_back(pos);
				}

				std::vector<Vec2>& uvs=p.uvs;
				for(auto& uv : pau.uvs){
					uvs.push_back(uv);
				}

				p.position=pos+pau.center;
				auto grave=PhysicsBody::createPolygon(&v[0],v.size(),_pm);
				DrawNodeWithTex* node = DrawNodeWithTex::create();
				

				auto tex=Director::getInstance()->
					getTextureCache()->
					addImage("ice.png");
					//addImage("soniko_t.png");
				node->SetTexture(tex->getName());
				node->drawPolygonT(&v[0],&uvs[0],v.size(),cocos2d::Color4F::YELLOW,0,Color4F::WHITE);
				node->setPosition(p.position);
				node->setPhysicsBody(grave);
				node->setTag(_tag++);

				DrawNode* border=DrawNode::create();
				border->drawPolygon(&v[0],v.size(),Color4F(0,0,0,0.0),BORDER_WIDTH,Color4F::WHITE);
				node->addChild(border);

				p.tag=node->getTag();
				p.texId = tex->getName();
				p.wasCut=false;
				_polygons.push_back(p);
				addChild(node,priority++);
			}
		}else if(type=="scaffold"){
			Size size;
			size.width=map["width"].asInt();
			size.height=map["height"].asInt();
			PhysicsMaterial bm;
			bm.density=1.0f;
			bm.friction=0.3f;
			bm.restitution=0.0f;

			auto board=PhysicsBody::createBox(size,bm);
			auto boardnode = Sprite::create("marble.png",Rect(0,0,size.width,size.height));
			auto rot = map.find("rotation");
			float angle=0;
			if(rot!=map.end()){
				angle = rot->second.asFloat();
			}
			
			boardnode->setPosition(pos+Vec2(size.width/2,size.height/2).rotate(Vec2::forAngle(-angle*M_PI/180.0f)));
			boardnode->setPhysicsBody(board);
			boardnode->setRotation(angle);
			board->setDynamic(false);
			addChild(boardnode);
		}
	}
}

void
GamePlaying::ImageLayerParse(TMXObjectGroup* og,int& priority ){
	if(og==nullptr){
		return;
	}
	auto objs = og->getObjects();

	int assertcount=0;
	for(auto& obj : objs){
		ValueMap& map = obj.asValueMap();
		Vec2 pos;
		
		std::string type = map["type"].asString();
		pos.x=map["x"].asFloat();
		pos.y=map["y"].asFloat();
		if(type=="cuttable"){	
			PossesAndUvs pau=GetPolygonVerticesInfo(map);
			if(!pau.posses.empty()){
				///ポリゴンの生成
				MyPolygon p;
				std::vector<Vec2>& v = p.vertices;
				for(auto& pos : pau.posses){
					v.push_back(pos);
				}

				std::vector<Vec2>& uvs=p.uvs;
				for(auto& uv : pau.uvs){
					uvs.push_back(uv);
				}

				p.position=pos+pau.center;
				auto grave=PhysicsBody::createPolygon(&v[0],v.size(),_pm);
				DrawNodeWithTex* node = DrawNodeWithTex::create();
			
				auto tex=Director::getInstance()->
					getTextureCache()->
					addImage("soniko.png");
				node->SetTexture(tex->getName());
				node->drawPolygonT(&v[0],&uvs[0],v.size(),cocos2d::Color4F::YELLOW,0,Color4F::WHITE);
				node->setPosition(p.position);
				node->setPhysicsBody(grave);
				node->setTag(_tag++);


				p.tag=node->getTag();
				p.texId = tex->getName();
				p.wasCut=false;
				_polygons.push_back(p);
				addChild(node,priority++);
			}
		}else if(type=="scaffold"){
			Size size;
			size.width=map["width"].asInt();
			size.height=map["height"].asInt();
			PhysicsMaterial bm;
			bm.density=1.0f;
			bm.friction=0.3f;
			bm.restitution=0.0f;

			auto board=PhysicsBody::createBox(size,bm);
			auto boardnode = Sprite::create("marble.png",Rect(0,0,size.width,size.height));
			auto rot = map.find("rotation");
			float angle=0;
			if(rot!=map.end()){
				angle = rot->second.asFloat();
			}
			
			boardnode->setPosition(pos+Vec2(size.width/2,size.height/2).rotate(Vec2::forAngle(-angle*M_PI/180.0f)));
			boardnode->setPhysicsBody(board);
			boardnode->setRotation(angle);
			board->setDynamic(false);
			addChild(boardnode);
		}
	}
}


cocos2d::LabelTTF* 
GamePlaying::CreateLocalizedLabel(const char* str,const char* fontname,int size){
#ifdef _WINDOWS
		return LabelTTF::create(str, fontname, size);
#else
		return  LabelTTF::create(GetUTF8FromSJIS(str),fontname,size);
#endif
}

// on "init" you need to initialize your instance
bool GamePlaying::init()
{
	_trans=false;
	_tag=0;
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
						CC_CALLBACK_1(GamePlaying::menuCloseCallback, this));
	Vec2 menupos = Vec2(origin.x + visibleSize.width,
				origin.y + closeItem->getContentSize().height/2);
	menupos.x -=closeItem->getContentSize().width/2;
	closeItem->setPosition(menupos);

	
	//リロード
	auto reloadItem = MenuItemImage::create(
		"reload.png",
		"reload.png",
		CC_CALLBACK_1(GamePlaying::menuReloadCallback, this));
	menupos.x =visibleSize.width/2;
	reloadItem->setPosition(menupos);

	//次へ
	auto nextItem = MenuItemImage::create(
		"next.png",
		"next.png",
		CC_CALLBACK_1(GamePlaying::menuNextCallback, this));
	
	menupos.x =visibleSize.width/2 + nextItem->getContentSize().width*2;
	nextItem->setPosition(menupos);


	// create menu, it's an autorelease object
	auto menu = Menu::create(closeItem, reloadItem ,nextItem, nullptr);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu, 100);

	int priority=0;


	_pm = PHYSICSSHAPE_MATERIAL_DEFAULT;
	_pm.friction = 0.5f;
	_pm.density = 0.5f;
	/////////////////////////////
	// 3. add your codes below...

	//背景
	auto bg = SpriteBatchNode::create("bg.png");
	auto bg2 = SpriteBatchNode::create("bg2.png");
	int i=0,j=0;
	for(int j=0;j<4;++j){
		for(int i=0;i<2;++i){
			auto sp=Sprite::createWithTexture(bg->getTexture());
			sp->setPosition(128+i*256,128+j*256);
			bg->addChild(sp);
			auto sp2=Sprite::createWithTexture(bg2->getTexture());
			sp2->setPosition(128+i*256,128+j*256);
			bg2->addChild(sp2);
			sp2->runAction(RepeatForever::create(Sequence::createWithTwoActions( FadeIn::create(2.5f),FadeOut::create(2.5f) )));
		}
	}
	addChild(bg);
	addChild(bg2);

	

	TMXTiledMap* tiledmap;
	TMXMapInfo* mapinfo;
	if(stage_no==0){
		_slushLimit=1;
		auto label = LabelTTF::create(GetUTF8FromSJIS("チャンスは１度きり…"), "Arial", 24);
		// position the label on the center of the screen
		label->setPosition(Vec2(origin.x + visibleSize.width/2,
					origin.y + visibleSize.height - label->getContentSize().height));
		// add the label as a child to this layer
		this->addChild(label, priority++);
		tiledmap = TMXTiledMap::create("stage1.tmx");
		mapinfo = TMXMapInfo::create("stage1.tmx");
	}else if(stage_no==2){
		_slushLimit=2;
		auto label = LabelTTF::create(GetUTF8FromSJIS("チャンスは2回のみだ"), "Arial", 24);
		// position the label on the center of the screen
		label->setPosition(Vec2(origin.x + visibleSize.width/2,
					origin.y + visibleSize.height - label->getContentSize().height));

		// add the label as a child to this layer
		this->addChild(label, priority++);

		tiledmap = TMXTiledMap::create("stage2.tmx");
		mapinfo = TMXMapInfo::create("stage2.tmx");
	}else if(stage_no==2){
		_slushLimit=3;
#ifdef _WINDOWS
		auto label = LabelTTF::create(GetUTF8FromSJIS("3回切ってくれたまえ"), "Arial", 24);
#else
		auto label = LabelTTF::create(GetUTF8FromSJIS("3回切ってくれたまえ"),"Arial",24);
#endif
		// position the label on the center of the screen
		label->setPosition(Vec2(origin.x + visibleSize.width/2,
					origin.y + visibleSize.height - label->getContentSize().height));

		// add the label as a child to this layer
		this->addChild(label, priority++);

		tiledmap = TMXTiledMap::create("stage3.tmx");
		mapinfo = TMXMapInfo::create("stage3.tmx");
	}else{
		_slushLimit=3;
		auto data = cocos2d::FileUtils::sharedFileUtils()->getStringFromFile("String.txt");
		
		auto label = LabelTTF::create(data.c_str()/*("3回ぶった斬れ!!")*/, "MS Pゴシック", 24);
    
		// position the label on the center of the screen
		label->setPosition(Vec2(origin.x + visibleSize.width/2,
					origin.y + visibleSize.height - label->getContentSize().height));

		// add the label as a child to this layer
		this->addChild(label, priority++);

		tiledmap = TMXTiledMap::create("stage4.tmx");
		mapinfo = TMXMapInfo::create("stage4.tmx");
	}
	
	auto soni=Sprite::create("soniko_t.png");
	soni->setScale(0.5);
	soni->setPosition(250,400);
	addChild(soni);


	Vector<TMXTilesetInfo*>& tileset = mapinfo->getTilesets();
	TMXTilesetInfo* ts= tileset.at(0);
	ts->_sourceImage;
	
	ObjectLayerParse(tiledmap->getObjectGroup("cuttable"),priority);
	ObjectLayerParse(tiledmap->getObjectGroup("scaffold"),priority);
  
	auto director = Director::getInstance();
	
	auto listener=EventListenerTouchOneByOne::create();
	listener->onTouchBegan=CC_CALLBACK_2(GamePlaying::onTouchBegan,this);
	listener->onTouchMoved=CC_CALLBACK_2(GamePlaying::onTouchMoved,this);
	listener->onTouchEnded=CC_CALLBACK_2(GamePlaying::onTouchEnded,this);

	getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener,this);

	Director::getInstance()->getTextureCache()->addImage("blade.png");

	auto qp=ParticleSystemQuad::create("hellfire.plist");
	qp->setPosition(300,0);
	addChild(qp,10);

	//auto streak = MotionStreak::create(0.3f,0.01f,30.f,Color3B::WHITE,"trail.png");
	//streak->setName("streak");
	//streak->setBlendFunc(BlendFunc::ALPHA_PREMULTIPLIED);
	//addChild(streak);

	//streak->setBlendFunc(b);
	//cocos2d::BlendFunc b;
	//b.dst=GL_ZERO;
	//b.src=GL_ONE_MINUS_DST_COLOR;
	//
	//auto s=Sprite::create();
	//s->setTextureRect(Rect(0,0,480,800));
	//s->setPosition(240,400);
	//s->setColor(Color3B::WHITE);
	//s->setBlendFunc(b);
	//s->setName("reverse");
	//s->setVisible(false);
	//addChild(s,65535);

	scheduleUpdate();

    return true;
}

void 
GamePlaying::menuNextCallback(cocos2d::Ref* pSender){
	stage_no = (stage_no==3?0:stage_no+1);
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f,GamePlaying::createScene()));
}

void GamePlaying::menuReloadCallback(cocos2d::Ref* pSender){
	Director::getInstance()->replaceScene(TransitionFade::create(1.0f,GamePlaying::createScene()));
}


void GamePlaying::menuCloseCallback(Ref* pSender)
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


bool 
GamePlaying::onTouchBegan(Touch* t,Event* ev){
	if(_slushLimit==0){
		return true;
	}
	//auto s = getChildByName("reverse");
	//s->setVisible(true);
	//s->runAction(Sequence::createWithTwoActions(DelayTime::create(0.5f),CallFunc::create([s](){s->setVisible(false);})));

	_touchedPos = t->getLocation();
	auto sprite=Sprite::create("blade.png");
	sprite->setAnchorPoint(Vec2(0.f,0.5f));
	sprite->setScaleY(0.1f);
	sprite->setScaleX(0);
	sprite->setName("stigma");
	sprite->setPosition(_touchedPos);
	addChild(sprite,100);

	//auto streak = getChildByName("streak");
	//streak->setPosition(t->getLocation());

	return true;
}

void
GamePlaying::onTouchMoved(Touch* t,Event* ev){
	if(_slushLimit==0){
		return;
	}
	auto stigma = getChildByName("stigma");
	auto pos = t->getLocation();
	auto v=(pos-_touchedPos);
	stigma->setScaleX(v.length()/512.0f);
	stigma->setRotation(angle2d(v,Vec2(1,0))*180.0f/3.14159265358979);
	
	//auto streak = getChildByName("streak");
	//streak->setPosition(t->getLocation());

}

class MyPhysicsShape : public cocos2d::PhysicsShape{
public:
	cpPolyShape* getPolyShape(){
		return (cpPolyShape*)(_cpShapes[0]);
	}
};

void 
GamePlaying::CreateSlushParticle(cocos2d::Vec2& startpos,cocos2d::Vec2& endpos){
	//auto ptcl=ParticleFireworks::create();
	auto ptcl=ParticleSystemQuad::create("ice.plist");

	//ptcl->setAutoRemoveOnFinish(true);
	//ptcl->setEmissionRate(200);
	//ptcl->setColor(Color3B::WHITE);
	//ptcl->setStartColor(Color4F::WHITE);
	//ptcl->setEndColor(Color4F::WHITE);
	ptcl->setPosition(startpos);
	ptcl->runAction(Sequence::createWithTwoActions(MoveTo::create(0.5f,endpos),RemoveSelf::create()));
	addChild(ptcl,10000);
}

void 
GamePlaying::onTouchEnded(Touch* t,Event* ev){
	if(_slushLimit==0){
		return;
	}

	auto stigma = getChildByName("stigma");
	stigma->setName("");
	stigma->runAction(Sequence::createWithTwoActions(FadeOut::create(0.5f),RemoveSelf::create()));

	//CreateSlushParticle(_touchedPos,t->getLocation());

	bool flg=false;
	///カットチェック
	for(MyPolygon& p : _polygons){
		auto nnode = getChildByTag(p.tag);
		if(nnode==nullptr){
			continue;
		}
		auto psp=(MyPhysicsShape*)(nnode->getPhysicsBody()->getFirstShape());
		auto cps=(cpPolyShape*)(psp->getPolyShape());
		
		auto& polyPos = getChildByTag(p.tag)->getPosition();
		for(int j=0;j<cps->numVerts;++j){
			if(j==cps->numVerts-1){
				flg=IsIntersected(_touchedPos,
					t->getLocation(),
					Vec2(cps->verts[j].x,cps->verts[j].y)+polyPos,
					Vec2(cps->verts[0].x,cps->verts[0].y)+polyPos);
				if(flg){
					PosUv posuv;
					posuv.pos = IntersectedPoint(_touchedPos,
								t->getLocation(),
								Vec2(cps->verts[j].x,cps->verts[j].y)+polyPos,
								Vec2(cps->verts[0].x,cps->verts[0].y)+polyPos);
					//とりあえずvertsのインデックスとp.vertices,p.uvsのインデクスが同一であると仮定する。
					posuv.uv = GetMidUV(Vec2(cps->verts[j].x,cps->verts[j].y)+polyPos,
						p.uvs[j],
						Vec2(cps->verts[0].x,cps->verts[0].y)+polyPos,
						p.uvs[0],
						posuv.pos);
					p.cutEdges.push_back(std::make_pair(j,posuv));//インデックスとuv&posのペア
					p.wasCut=true;
					
				}
			}else{
				flg=IsIntersected(_touchedPos,
					t->getLocation(),
					Vec2(cps->verts[j].x,cps->verts[j].y)+polyPos,
					Vec2(cps->verts[j+1].x,cps->verts[j+1].y)+polyPos);
				if(flg){
					PosUv posuv;
					posuv.pos =IntersectedPoint(_touchedPos,
								t->getLocation(),
								Vec2(cps->verts[j].x,cps->verts[j].y)+polyPos,
								Vec2(cps->verts[j+1].x,cps->verts[j+1].y)+polyPos);
					//とりあえずvertsのインデックスとp.vertices,p.uvsのインデクスが同一であると仮定する。
					posuv.uv = GetMidUV(Vec2(cps->verts[j].x,cps->verts[j].y)+polyPos,
						p.uvs[j],
						Vec2(cps->verts[j+1].x,cps->verts[j+1].y)+polyPos,
						p.uvs[j+1],
						posuv.pos);
					p.cutEdges.push_back(std::make_pair(j,posuv));
					p.wasCut=true;

				}
			}
		}
	}

	std::vector<Vec2> cuttingEdges;

	///一時的…追加用ベクタ作成
	std::vector<MyPolygon> addedpolygons(0);
	int cutcount=0;
	///分離処理(頂点の生成)
	for(MyPolygon& p : _polygons){
		if(p.cutEdges.empty() || p.cutEdges.size()==1){//カットできてない場合は次へ
			p.wasCut=false;
			p.cutEdges.clear();
			continue;
		}
		++cutcount;
		//n番目がカットされている場合、nとn+1の間をカットしている
		//カットされている場合は2つのカットポイントが存在する

		Vec2& fpos=p.cutEdges[0].second.pos;
		Vec2& spos=p.cutEdges[1].second.pos;

		if((_touchedPos-fpos).length()>(_touchedPos-spos).length()){
			//切るエフェクトの発生
			CreateSlushParticle(spos,fpos);
		}else{
			//切るエフェクトの発生
			CreateSlushParticle(fpos,spos);
		}

		//とりあえず複製を一つ用意する

		//頂点数も変わる。エッジ番号の差が1なら残りの頂点数は1つまり差=残り頂点数となる

		//↑2つが新たな頂点となる(ソートが必要)
		if(p.cutEdges[0].first > p.cutEdges[1].first){
			std::swap(p.cutEdges[0],p.cutEdges[1]);
		}

		auto nnode = getChildByTag(p.tag);
		auto& polyPos = nnode->getPosition();
		auto psp=(MyPhysicsShape*)(nnode->getPhysicsBody()->getFirstShape());
		auto cps=(cpPolyShape*)(psp->getPolyShape());

		//sort完了
		//差を算出
		int diff = p.cutEdges[1].first-p.cutEdges[0].first;
		Vec2 pos=p.position;
		MyPolygon poly;
		std::vector<Vec2>& tmpUvs=p.uvs;

		poly.vertices.clear();
		poly.uvs.clear();
		
		poly.tag = p.tag;
		poly.texId = p.texId;
		poly.wasCut=p.wasCut;

		Vec2 newpos = p.cutEdges[0].second.pos-polyPos;
		poly.vertices.push_back(newpos);
		poly.uvs.push_back(p.cutEdges[0].second.uv);//切断端点
		for(int i=1;i<=diff;++i){
			int idx=p.cutEdges[0].first+i;
			cpVect& ve=cps->verts[idx];//元の座標を利用
			newpos = Vec2(ve.x,ve.y);
			poly.vertices.push_back(newpos);
			poly.uvs.push_back(tmpUvs[idx]);
		}
		newpos = p.cutEdges[1].second.pos-polyPos;
		poly.vertices.push_back(newpos);
		poly.uvs.push_back(p.cutEdges[1].second.uv);//切断端点
		poly.position = polyPos;
		addedpolygons.push_back(poly);

		poly.vertices.clear();
		poly.uvs.clear();

		//もうひとつのポリゴン
		diff=p.vertices.size()-diff;
		poly.texId = p.texId;
		poly.wasCut=p.wasCut;
		newpos = p.cutEdges[1].second.pos-polyPos;
		poly.vertices.push_back(newpos);
		poly.uvs.push_back(p.cutEdges[1].second.uv);//切断端点
		for(int i=1;i<=diff;++i){
			int idx = (p.cutEdges[1].first+i)%p.vertices.size();
			cpVect& ve=cps->verts[idx];//元の座標を利用
			newpos = Vec2(ve.x,ve.y);
			poly.vertices.push_back(newpos);
			poly.uvs.push_back(tmpUvs[idx]);
		}
		newpos = p.cutEdges[0].second.pos-polyPos;
		poly.vertices.push_back(newpos);
		poly.uvs.push_back(p.cutEdges[0].second.uv);//切断端点
		poly.position = polyPos;
		addedpolygons.push_back(poly);
	}

	//切れてなければリターンする
	if(cutcount==0){
		removeChild(stigma);
		return;
	}

	for(auto& p : _polygons){
		auto node = getChildByTag(p.tag);
		if(node==nullptr){
			continue;
		}
		auto& polyPos = node->getPosition();
		if(!p.wasCut){
			p.position=polyPos;
		}
	}

	//切れてしまっている奴は、別の２つに生まれ変わっているので、元のは消す。
	for(auto& p : _polygons){
		if(p.wasCut){
			removeChild(getChildByTag(p.tag));
		}
	}

	class IFunction{
	public:
		bool operator()(MyPolygon& a){
			return a.wasCut;
		}
	};

	_polygons.erase(std::remove_if(_polygons.begin(),_polygons.end(),IFunction()),_polygons.end());

	std::copy(addedpolygons.begin(),addedpolygons.end(),std::back_inserter(_polygons));
	
	///実際のポリゴン生成
	for(auto& pl : _polygons){
		if(!pl.wasCut){
			continue;
		}
		auto& v=pl.vertices;
		auto& uvs=pl.uvs;
		auto node = DrawNodeWithTex::create();
		node->SetTexture(pl.texId);
		//重心の計算(疑似)
		Vec2 center=Vec2(0,0);
		for(auto& vert : v){
			center+=vert;
		}
		center = center/v.size();
		for(auto& vert : v){
			vert-=center;
		}

		auto grave=PhysicsBody::createPolygon(&v[0],v.size(),_pm);

		node->drawPolygonT(&v[0],&uvs[0],v.size(),cocos2d::Color4F::YELLOW,0,Color4F::WHITE);
		
		for(Vec2&  uv : uvs){
			log("u=%f,v=%f",uv.x,uv.y);
		}

		pl.position+=center;
		node->setPosition(pl.position);
		node->setPhysicsBody(grave);
		node->setTag(_tag++);
		pl.tag=node->getTag();
		pl.wasCut=false;
		addChild(node);
		DrawNode* border=DrawNode::create();
		border->drawPolygon(&v[0],v.size(),Color4F(0,0,0,0.0),BORDER_WIDTH,Color4F::WHITE);
		node->addChild(border);
	}
	--_slushLimit;
}


bool GamePlaying::IsIntersected(const Vec2& firstA,const Vec2& secondA,
		const Vec2& firstB,const Vec2& secondB){
			return ((secondA-firstA).cross(firstB-firstA)*(secondA-firstA).cross(secondB-firstA) <= 0 && 
				(secondB-firstB).cross(firstA-firstB)*(secondB-firstB).cross(secondA-firstB) <= 0 );
}

Vec2 GamePlaying::IntersectedPoint(const cocos2d::Vec2& firstA,const cocos2d::Vec2& secondA,
		const cocos2d::Vec2& firstB,const cocos2d::Vec2& secondB){
			
			Vec2 ab=secondA-firstA;
			Vec2 ac=firstB-firstA;
			Vec2 ad=secondB-firstA;
			Vec2 cd=secondB-firstB;

			Vec2 pos = firstB + cd * (fabs(ab.cross(ac))/(fabs(ab.cross(ac))+fabs(ab.cross(ad))));

			return pos;
}
