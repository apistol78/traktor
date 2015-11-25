#ifndef traktor_render_EAGLContextWrapper_H
#define traktor_render_EAGLContextWrapper_H

#include "Render/OpenGL/Platform.h"

#ifdef __OBJC__
typedef CAEAGLLayer CAEAGLLayer_t;
typedef EAGLContext EAGLContext_t;
#else
typedef void CAEAGLLayer_t;
typedef void EAGLContext_t;
#endif

namespace traktor
{
	namespace render
	{

class EAGLContextWrapper
{
public:
	EAGLContextWrapper();
	
	bool create();
	
	bool create(void* nativeHandle);
	
	void destroy();
	
	static bool setCurrent(EAGLContextWrapper* context);
	
	void swapBuffers();
	
	void resize(GLint width, GLint height);

	static bool getLandscape();

	GLuint getFrameBuffer() const { return m_frameBuffer; }

	GLuint getDepthBuffer() const { return m_depthBuffer; }
	
	GLint getWidth() const { return m_width; }
	
	GLint getHeight() const { return m_height; }

	static int32_t getCurrentWidth();

	static int32_t getCurrentHeight();
	
private:
	CAEAGLLayer_t* m_layer;
	EAGLContext_t* m_context;
	GLuint m_frameBuffer;
	GLuint m_renderBuffer;
	GLuint m_depthBuffer;
	GLint m_width;
	GLint m_height;
	
	void createFrameBuffer();
	
	void destroyFrameBuffer();
};

	}
}

#endif	// traktor_render_EAGLContextWrapper_H
