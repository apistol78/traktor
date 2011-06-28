#ifndef traktor_render_BlitHelper_H
#define traktor_render_BlitHelper_H

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

#if !defined(T_OFFLINE_ONLY)

class ContextOpenGLES2;
class StateCache;

class BlitHelper : public Object
{
	T_RTTI_CLASS;
	
public:
	bool create(ContextOpenGLES2* resourceContext);
	
	void destroy();

	void blit(StateCache* stateCache, GLint sourceTextureHandle);
	
private:
	GLuint m_programObject;
	GLint m_attribPosition;
	GLint m_attribTexCoord;
	GLint m_samplerIndex;
	GLuint m_vertexBuffer;
	RenderState m_renderState;
};
        
#endif
	
	}
}

#endif	// traktor_render_BlitHelper_H
