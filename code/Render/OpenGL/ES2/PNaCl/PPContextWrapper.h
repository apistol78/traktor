#ifndef traktor_render_PPContextWrapper_H
#define traktor_render_PPContextWrapper_H

#include <ppapi/cpp/graphics_3d.h>
#include <ppapi/cpp/graphics_3d_client.h>
#include "Core/Object.h"
#include "Core/Thread/Signal.h"

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
	static Ref< PPContextWrapper > createResourceContext(pp::Instance* instance);

	static Ref< PPContextWrapper > createRenderContext(pp::Instance* instance, PPContextWrapper* shareContext);

	virtual ~PPContextWrapper();

	bool resize(int32_t width, int32_t height);

	int32_t getWidth() const;

	int32_t getHeight() const;

	bool makeCurrent();

	void swapBuffers();

	void _doSwapBuffers();

	void _doSwapBuffersFinished();

private:
	pp::Graphics3D m_context;
	Signal m_swap;

	PPContextWrapper();
};

	}
}

#endif	// traktor_render_PPContextWrapper_H
