#ifndef __GamePlaying_SCENE_H__
#define __GamePlaying_SCENE_H__

#include "cocos2d.h"
#include<vector>

struct PosUv{
	cocos2d::Vec2 pos;//�ʒu
	cocos2d::Vec2 uv;//UV
};

typedef std::vector<PosUv> PosUvs_t;

struct PossesAndUvs{
	cocos2d::Vec2 center;
	std::vector<cocos2d::Vec2> posses;//�ʒu(����)
	std::vector<cocos2d::Vec2> uvs;//UV(����)
};

struct MyPolygon{
	int tag;
	GLuint texId;
	bool wasCut;
	cocos2d::Vec2 position;
	///@doing vertices��uvs����������
	std::vector<cocos2d::Vec2> vertices;
	std::vector<cocos2d::Vec2> uvs;
	std::vector<std::pair<int,PosUv>> cutEdges;
};

class GamePlaying : public cocos2d::Layer
{ 
private:
	int _slushCount;//�؂�����
	int _slushLimit;//�؂����E
	int _tag;//���݂̃^�O�ő�l
	std::vector<MyPolygon> _polygons;//�|���S�����z��
	cocos2d::Vec2 _touchedPos;//�^�b�`�ς݃|�W�V����

	//��������
	//@retval true �������Ă���
	//@retval false �������Ă��Ȃ�
	bool IsIntersected(const cocos2d::Vec2& firstA,const cocos2d::Vec2& secondA,
		const cocos2d::Vec2& firstB,const cocos2d::Vec2& secondB);

	//�����|�C���g
	//@return 
	cocos2d::Vec2 IntersectedPoint(const cocos2d::Vec2& firstA,const cocos2d::Vec2& secondA,
		const cocos2d::Vec2& firstB,const cocos2d::Vec2& secondB);
	cocos2d::PhysicsMaterial _pm;

	///TiledMapper�̃}�b�v��񂩂瑽�p�`���(�ꏊ��UV)�𐶐����ĕԂ�
	///@param map �}�b�v���
	///@return ���p�`�̒��_���(UV����)
	///@attention �������}�b�v��type=cuttable�ȉ��̃}�b�v�ł��邱��
	PossesAndUvs GetPolygonVerticesInfo(cocos2d::ValueMap& map);


	///TiledMapper�̃}�b�v��񂩂�y���`���𐶐�����node(Physics��)��Ԃ�
	///@param map �}�b�v���
	///@return �m�[�h
	///@attention �������}�b�v��type=scaffold�ȉ��̃}�b�v�ł��邱��
	cocos2d::Node* CreateScaffoldRectNode(cocos2d::ValueMap& map);

	//�I�u�W�F�N�g���C���[���p�[�X����
	void ObjectLayerParse(cocos2d::TMXObjectGroup* og,int& priority );

	//�C���[�W���C���[���p�[�X����
	void ImageLayerParse(cocos2d::TMXObjectGroup* og,int& priority );


	//���[�J���C�Y�ς݂̃��x����\������
	cocos2d::LabelTTF* CreateLocalizedLabel(const char* str,const char* fontname,int size);


	//�s�v�ȃ|���S��(��ʊO�ɏo�������)���폜����
	void CheckAndRemovePolygons();

	//�S�Ẵ|���S���̓������~�܂��Ă��邩�ǂ������`�F�b�N
	//@retval true �S�Ă��~�܂��Ă���
	//@retval false �ǂꂩ�������Ă���
	bool CheckAllPolygonsStopped();

	//�ЂƂł��c�u�|���S�����c���Ă��邩�ǂ���
	//retval true �ǂ����Ƀ|���S�����c���Ă���
	//retval false �|���S���͑S�ăS�~���ɍs���Ă���
	bool CheckRemainAnyPolygons();

	//�؂������̃G�t�F�N�g����
	//@param startpos �n�_
	//@param endpos �I�_
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
