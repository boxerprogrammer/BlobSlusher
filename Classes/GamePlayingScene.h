#ifndef __GamePlaying_SCENE_H__
#define __GamePlaying_SCENE_H__

#include "cocos2d.h"
#include<vector>

struct PosUv{
	cocos2d::Vec2 pos;//位置
	cocos2d::Vec2 uv;//UV
};

typedef std::vector<PosUv> PosUvs_t;

struct PossesAndUvs{
	cocos2d::Vec2 center;
	std::vector<cocos2d::Vec2> posses;//位置(複数)
	std::vector<cocos2d::Vec2> uvs;//UV(複数)
};

struct MyPolygon{
	int tag;
	GLuint texId;
	bool wasCut;
	cocos2d::Vec2 position;
	///@doing verticesとuvsを混合する
	std::vector<cocos2d::Vec2> vertices;
	std::vector<cocos2d::Vec2> uvs;
	std::vector<std::pair<int,PosUv>> cutEdges;
};

class GamePlaying : public cocos2d::Layer
{ 
private:
	int _slushCount;//切った回数
	int _slushLimit;//切れる限界
	int _tag;//現在のタグ最大値
	std::vector<MyPolygon> _polygons;//ポリゴン情報配列
	cocos2d::Vec2 _touchedPos;//タッチ済みポジション

	//交差判定
	//@retval true 交差している
	//@retval false 交差していない
	bool IsIntersected(const cocos2d::Vec2& firstA,const cocos2d::Vec2& secondA,
		const cocos2d::Vec2& firstB,const cocos2d::Vec2& secondB);

	//交差ポイント
	//@return 
	cocos2d::Vec2 IntersectedPoint(const cocos2d::Vec2& firstA,const cocos2d::Vec2& secondA,
		const cocos2d::Vec2& firstB,const cocos2d::Vec2& secondB);
	cocos2d::PhysicsMaterial _pm;

	///TiledMapperのマップ情報から多角形情報(場所とUV)を生成して返す
	///@param map マップ情報
	///@return 多角形の頂点情報(UVあり)
	///@attention 代入するマップはtype=cuttable以下のマップであること
	PossesAndUvs GetPolygonVerticesInfo(cocos2d::ValueMap& map);


	///TiledMapperのマップ情報から土台矩形情報を生成してnode(Physicsつき)を返す
	///@param map マップ情報
	///@return ノード
	///@attention 代入するマップはtype=scaffold以下のマップであること
	cocos2d::Node* CreateScaffoldRectNode(cocos2d::ValueMap& map);

	//オブジェクトレイヤーをパースする
	void ObjectLayerParse(cocos2d::TMXObjectGroup* og,int& priority );

	//イメージレイヤーをパースする
	void ImageLayerParse(cocos2d::TMXObjectGroup* og,int& priority );


	//ローカライズ済みのラベルを表示する
	cocos2d::LabelTTF* CreateLocalizedLabel(const char* str,const char* fontname,int size);


	//不要なポリゴン(画面外に出ちゃった)を削除する
	void CheckAndRemovePolygons();

	//全てのポリゴンの動きが止まっているかどうかをチェック
	//@retval true 全てが止まっている
	//@retval false どれかが動いている
	bool CheckAllPolygonsStopped();

	//ひとつでも残置ポリゴンが残っているかどうか
	//retval true どこかにポリゴンが残っている
	//retval false ポリゴンは全てゴミ箱に行っている
	bool CheckRemainAnyPolygons();

	//切った時のエフェクト生成
	//@param startpos 始点
	//@param endpos 終点
	void CreateSlushParticle(cocos2d::Vec2& startpos,cocos2d::Vec2& endpos);

	bool _trans;
public:
	bool onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* ev);
	void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* ev);
	void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* ev);
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();  
    
	// a selector callback
	void menuCloseCallback(cocos2d::Ref* pSender);

	void menuReloadCallback(cocos2d::Ref* pSender);

	void menuNextCallback(cocos2d::Ref* pSender);
    
	void update(float delta);

	// implement the "static create()" method manually
	CREATE_FUNC(GamePlaying);
};

#endif // __GamePlaying_SCENE_H__
