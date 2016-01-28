#ifndef traktor_render_PPContextWrapper_H
#define traktor_render_PPContextWrapper_H

#include <ppapi/cpp/graphics_3d.h>
#include <ppapi/cpp/graphics_3d_client.h>
#include "Core/Object.h"
#include "Core/Thread/Signal.h"
#include "Render/OpenGL/Platform.h"

namespace pp
{

class Instance;

}

namespace traktor
{
	namespace render
	{

class PPContextWrapper : public Object
{
	T_RTTI_CLASS;

public:
	static Ref< PPContextWrapper > createRenderContext(pp::Instance* instance);

	virtual ~PPContextWrapper();

	bool resize(int32_t width, int32_t height);

	int32_t getWidth() const;

	int32_t getHeight() const;

	bool makeCurrent();

	void swapBuffers();

	void _doSwapBuffers();

	void _doSwapBuffersFinished();

	GLuint getFrameBuffer() const { return m_frameBuffer; }

	GLuint getDepthBuffer() const { return m_depthBuffer; }

private:
	pp::Graphics3D m_context;
	Signal m_swap;
	int32_t m_width;
	int32_t m_height;

	GLuint m_frameBuffer;
	GLuint m_renderBuffer;
	GLuint m_depthBuffer;

	PPContextWrapper();
};

	}
}

#endif	// traktor_render_PPContextWrapper_H
