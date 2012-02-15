#ifndef traktor_render_StateCacheOpenGL_H
#define traktor_render_StateCacheOpenGL_H

#include "Core/Object.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class StateCacheOpenGL : public Object
{
	T_RTTI_CLASS;
	
public:
	StateCacheOpenGL();

	void forceRenderState(const RenderState& renderState);
	
	void setRenderState(const RenderState& renderState);
	
	void setColorMask(uint32_t colorMask);
	
	void setDepthMask(GLboolean depthMask);

	void setPermitDepth(bool permitDepth);
	
	//void setArrayBuffer(GLint arrayBuffer);

	//void setElementArrayBuffer(GLint elemArrayBuffer);
	//
	//void setVertexArrayObject(GLint vertexArrayObject);
	//
	//void setProgram(GLuint program);

private:
	RenderState m_renderState;
	bool m_permitDepth;
	//GLint m_arrayBuffer;
	//GLint m_elemArrayBuffer;
	//GLint m_vertexArrayObject;
	//GLuint m_program;
};
	
	}
}

#endif	// traktor_render_StateCacheOpenGL_H
