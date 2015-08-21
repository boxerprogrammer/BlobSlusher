#include "DrawNodeWithTex.h"
USING_NS_CC;

// Vec2 == CGPoint in 32-bits, but not in 64-bits (OS X)
// that's why the "v2f" functions are needed
static Vec2 v2fzero(0.0f,0.0f);

static inline Vec2 v2f(float x, float y)
{
    Vec2 ret(x, y);
	return ret;
}

static inline Vec2 v2fadd(const Vec2 &v0, const Vec2 &v1)
{
	return v2f(v0.x+v1.x, v0.y+v1.y);
}

static inline Vec2 v2fsub(const Vec2 &v0, const Vec2 &v1)
{
	return v2f(v0.x-v1.x, v0.y-v1.y);
}

static inline Vec2 v2fmult(const Vec2 &v, float s)
{
	return v2f(v.x * s, v.y * s);
}

static inline Vec2 v2fperp(const Vec2 &p0)
{
	return v2f(-p0.y, p0.x);
}

static inline Vec2 v2fneg(const Vec2 &p0)
{
	return v2f(-p0.x, - p0.y);
}

static inline float v2fdot(const Vec2 &p0, const Vec2 &p1)
{
	return  p0.x * p1.x + p0.y * p1.y;
}

static inline Vec2 v2fforangle(float _a_)
{
	return v2f(cosf(_a_), sinf(_a_));
}

static inline Vec2 v2fnormalize(const Vec2 &p)
{
	Vec2 r = Vec2(p.x, p.y).getNormalized();
	return v2f(r.x, r.y);
}

static inline Vec2 __v2f(const Vec2 &v)
{
//#ifdef __LP64__
	return v2f(v.x, v.y);
// #else
// 	return * ((Vec2*) &v);
// #endif
}

static inline Tex2F __t(const Vec2 &v)
{
	return *(Tex2F*)&v;
}


bool 
DrawNodeWithTex::init(){
	_blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

	//元々SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR(テクスチャなし)であるのをSHADER_NAME_POSITION_TEXTURE(テクスチャあり)に変更
	setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE));
	//setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_LENGTH_TEXTURE_COLOR));
    
	ensureCapacity(512);
    
	if (Configuration::getInstance()->supportsShareableVAO())
	{
	glGenVertexArrays(1, &_vao);
	GL::bindVAO(_vao);
	}
    
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)* _bufferCapacity, _buffer, GL_STREAM_DRAW);
    
	glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, vertices));
    
	glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, colors));
    
	glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_TEX_COORD);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, texCoords));
    
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    
	if (Configuration::getInstance()->supportsShareableVAO())
	{
	GL::bindVAO(0);
	}
    
	CHECK_GL_ERROR_DEBUG();
    
	_dirty = true;
    
	#if CC_ENABLE_CACHE_TEXTURE_DATA
	// Need to listen the event only when not use batchnode, because it will use VBO
	auto listener = EventListenerCustom::create(EVENT_RENDERER_RECREATED, [this](EventCustom* event){
	/** listen the event that renderer was recreated on Android/WP8 */
	this->init();
	});

	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	#endif
	
	return true;
}


void DrawNodeWithTex::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	_customCommand.init(_globalZOrder);
	_customCommand.func = CC_CALLBACK_0(DrawNodeWithTex::onDraw, this, transform, flags);
	renderer->addCommand(&_customCommand);
}

void DrawNodeWithTex::onDraw(const Mat4 &transform, uint32_t flags)
{

	auto glProgram = getGLProgram();
	glProgram->use();
	glProgram->setUniformsForBuiltins(transform);
	GL::bindTexture2D(_textureID);
	GL::blendFunc(_blendFunc.src, _blendFunc.dst);

	if (_dirty)
	{
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(V2F_C4B_T2F)*_bufferCapacity, _buffer, GL_STREAM_DRAW);
	_dirty = false;
	}
	if (Configuration::getInstance()->supportsShareableVAO())
	{
	GL::bindVAO(_vao);
	}
	else
	{
	GL::enableVertexAttribs(GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	// vertex
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, vertices));

	// color
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, colors));

	// texcood(UVあり)
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(V2F_C4B_T2F), (GLvoid *)offsetof(V2F_C4B_T2F, texCoords));
	}

	glDrawArrays(GL_TRIANGLES, 0, _bufferCount);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1,_bufferCount);
	CHECK_GL_ERROR_DEBUG();
}



void 
DrawNodeWithTex::drawPolygonT(Vec2 *verts, Vec2* uvs,int count, const Color4F &fillColor, float borderWidth, const Color4F &borderColor){
	CCASSERT(count >= 0, "invalid count value");

	//extrudevertsの最後にuvをくっつけている
	struct ExtrudeVerts {Vec2 offset, n , uv;};
	struct ExtrudeVerts* extrude = (struct ExtrudeVerts*)malloc(sizeof(struct ExtrudeVerts)*count);
	memset(extrude, 0, sizeof(struct ExtrudeVerts)*count);
	
	//UV付き頂点情報を生成
	for (int i = 0; i < count; i++)
	{
		Vec2 v0 = __v2f(verts[(i-1+count)%count]);
		Vec2 v1 = __v2f(verts[i]);
		Vec2 v2 = __v2f(verts[(i+1)%count]);
        
		Vec2 n1 = v2fnormalize(v2fperp(v2fsub(v1, v0)));
		Vec2 n2 = v2fnormalize(v2fperp(v2fsub(v2, v1)));
		
		Vec2 uv0 = __v2f(uvs[(i-1+count)%count]);
		Vec2 uv1 = __v2f(uvs[i]);
		Vec2 uv2 = __v2f(uvs[(i+1)%count]);

		Vec2 offset = v2fmult(v2fadd(n1, n2), 1.0/(v2fdot(n1, n2) + 1.0));
		struct ExtrudeVerts tmp = {offset, n2};
		extrude[i] = tmp;
	}
	
	//輪郭線を表示するか否か
	bool outline = (borderColor.a > 0.0 && borderWidth > 0.0);
	
	auto triangle_count = 3*count - 2;
	auto vertex_count = 3*triangle_count;
	ensureCapacity(vertex_count);
	
	V2F_C4B_T2F_Triangle *triangles = (V2F_C4B_T2F_Triangle *)(_buffer + _bufferCount);
	V2F_C4B_T2F_Triangle *cursor = triangles;
	
	//塗りつぶし？
	float inset = (outline == false ? 0.5 : 0.0);
	for (int i = 0; i < count-2; i++)
	{
		Vec2 v0 = v2fsub(__v2f(verts[0  ]), v2fmult(extrude[0  ].offset, inset));
		Vec2 v1 = v2fsub(__v2f(verts[i+1]), v2fmult(extrude[i+1].offset, inset));
		Vec2 v2 = v2fsub(__v2f(verts[i+2]), v2fmult(extrude[i+2].offset, inset));
		
		auto uv0 = __t(uvs[0]);
		auto uv1 = __t(uvs[i+1]);
		auto uv2 = __t(uvs[i+2]);

		V2F_C4B_T2F_Triangle tmp = {
			{v0, Color4B(fillColor), uv0},
			{v1, Color4B(fillColor), uv1},
			{v2, Color4B(fillColor), uv2},
		};

		*cursor++ = tmp;
	}
	
	for(int i = 0; i < count; i++)
	{
		int j = (i+1)%count;
		Vec2 v0 = __v2f(verts[i]);
		Vec2 v1 = __v2f(verts[j]);
		
		Vec2 n0 = extrude[i].n;
		
		Vec2 offset0 = extrude[i].offset;
		Vec2 offset1 = extrude[j].offset;
		
		if(outline)
	        {//輪郭線を描画する
			Vec2 inner0 = v2fsub(v0, v2fmult(offset0, borderWidth));
			Vec2 inner1 = v2fsub(v1, v2fmult(offset1, borderWidth));
			Vec2 outer0 = v2fadd(v0, v2fmult(offset0, borderWidth));
			Vec2 outer1 = v2fadd(v1, v2fmult(offset1, borderWidth));
			
			V2F_C4B_T2F_Triangle tmp1 = {
				{inner0, Color4B(borderColor), __t(v2fneg(n0))},
				{inner1, Color4B(borderColor), __t(v2fneg(n0))},
				{outer1, Color4B(borderColor), __t(n0)}
			};
			*cursor++ = tmp1;

			V2F_C4B_T2F_Triangle tmp2 = {
				{inner0, Color4B(borderColor), __t(v2fneg(n0))},
				{outer0, Color4B(borderColor), __t(n0)},
				{outer1, Color4B(borderColor), __t(n0)}
			};
			*cursor++ = tmp2;
		} else {
			Vec2 inner0 = v2fsub(v0, v2fmult(offset0, 0.5));
			Vec2 inner1 = v2fsub(v1, v2fmult(offset1, 0.5));
			Vec2 outer0 = v2fadd(v0, v2fmult(offset0, 0.5));
			Vec2 outer1 = v2fadd(v1, v2fmult(offset1, 0.5));
			
			V2F_C4B_T2F_Triangle tmp1 = {
				{inner0, Color4B(fillColor), __t(v2fzero)},
				{inner1, Color4B(fillColor), __t(v2fzero)},
				{outer1, Color4B(fillColor), __t(n0)}
			};
			*cursor++ = tmp1;

			V2F_C4B_T2F_Triangle tmp2 = {
				{inner0, Color4B(fillColor), __t(v2fzero)},
				{outer0, Color4B(fillColor), __t(n0)},
				{outer1, Color4B(fillColor), __t(n0)}
			};
			*cursor++ = tmp2;
		}
	}
	
	_bufferCount += vertex_count;
	
	_dirty = true;

	free(extrude);
}

DrawNodeWithTex::DrawNodeWithTex() : DrawNode()
{
}

DrawNodeWithTex::~DrawNodeWithTex()
{
    free(_buffer);
    _buffer = nullptr;
    
    glDeleteBuffers(1, &_vbo);
    _vbo = 0;
    
    if (Configuration::getInstance()->supportsShareableVAO())
    {
        glDeleteVertexArrays(1, &_vao);
        GL::bindVAO(0);
        _vao = 0;
    }
}

DrawNodeWithTex* DrawNodeWithTex::create()
{
    DrawNodeWithTex* ret = new DrawNodeWithTex();
    if (ret && ret->init())
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    
    return ret;
}


