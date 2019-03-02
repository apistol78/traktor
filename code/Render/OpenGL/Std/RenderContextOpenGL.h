#pragma once

#include "Core/Containers/SmallMap.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"

namespace traktor
{
	namespace render
	{

class ProgramOpenGL;
class ResourceContextOpenGL;

class RenderContextOpenGL : public ContextOpenGL
{
	T_RTTI_CLASS;

public:
#if defined(_WIN32)
	RenderContextOpenGL(ResourceContextOpenGL* resourceContext, HWND hWnd, HDC hDC, HGLRC hRC);
#elif defined(__APPLE__)
	RenderContextOpenGL(ResourceContextOpenGL* resourceContext, void* context);
#elif defined(__LINUX__)
	RenderContextOpenGL(ResourceContextOpenGL* resourceContext, ::Display* display, ::Window window, GLXContext context);
#endif

#if !defined(__LINUX__)
	void update();
#else
	void update(int32_t width, int32_t height);
#endif

	void swapBuffers(int32_t waitVBlanks);

	bool programActivate(const ProgramOpenGL* program);

	void bindRenderStateObject(uint32_t renderStateObject);

	void bindSamplerStateObject(uint32_t samplerStateObject, uint32_t stage, bool haveMips);

	bool bindVertexArrayObject(uint32_t vertexBufferId);

	void setPermitDepth(bool permitDepth);

	int32_t getPhysicalWidth() const { return m_width; }

	int32_t getPhysicalHeight() const { return m_height; }

private:
	Ref< ResourceContextOpenGL > m_resourceContext;
	int32_t m_width;
	int32_t m_height;
	bool m_permitDepth;
	const ProgramOpenGL* m_currentProgram;
	uint32_t m_currentRenderStateList;
	int32_t m_lastWaitVBlanks;
	// GLuint m_vertexArrayObjects[4096];
	SmallMap< uint32_t, GLuint > m_vertexArrayObjects;
};

	}
}
