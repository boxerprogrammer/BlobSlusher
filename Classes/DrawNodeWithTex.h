#pragma once
#include <cocos2d.h>
class DrawNodeWithTex :	public cocos2d::DrawNode
{
private:
	GLuint _textureID;
public:
	void SetTexture(GLuint textureid){_textureID=textureid;}
	GLuint GetTexture(){return _textureID;}
	void onDraw(const cocos2d::Mat4 &transform, uint32_t flags);
	virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags) override;

	virtual bool init();
	static DrawNodeWithTex* create();
	void drawPolygonT(cocos2d::Vec2 *verts,cocos2d::Vec2* uvs, int count, const cocos2d::Color4F &fillColor, float borderWidth, const cocos2d::Color4F &borderColor);
	DrawNodeWithTex(void);
	virtual ~DrawNodeWithTex(void);
};

