#ifndef traktor_render_EAGLContextWrapper_H
#define traktor_render_EAGLContextWrapper_H

#include "Render/OpenGL/Platform.h"

namespace traktor
{
	namespace render
	{

class EAGLContextWrapper
{
public:
	EAGLContextWrapper();
	
	bool create();
	
	bool create(EAGLContextWrapper* shareContext, void* nativeHandle);
	
	void destroy();
	
	static bool setCurrent(EAGLContextWrapper* context);
	
	void swapBuffers();
	
	void resize(GLint width, GLint height);

	bool getLandscape() const;

	GLuint getFrameBuffer() const { return m_frameBuffer; }

	GLuint getDepthBuffer() const { return m_depthBuffer; }
	
	GLint getWidth() const { return m_width; }
	
	GLint getHeight() const { return m_height; }

	static int32_t getCurrentWidth();

	static int32_t getCurrentHeight();
	
private:
	void* m_layer;
	void* m_context;
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
