#include <ppapi/cpp/message_loop.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/instance_handle.h>
#include <ppapi/cpp/module.h>
#include <ppapi/gles2/gl2ext_ppapi.h>
#include "Core/Log/Log.h"
#include "Render/OpenGL/ES2/PNaCl/PPContextWrapper.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const int32_t c_defaultWidth = 640;
const int32_t c_defaultHeight = 480;

void deferredCallback(void* data, int32_t result)
{
	static_cast< PPContextWrapper* >(data)->_doSwapBuffers();
}

void flushCallback(void* data, int32_t result)
{
	static_cast< PPContextWrapper* >(data)->_doSwapBuffersFinished();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.PPContextWrapper", PPContextWrapper, Object)

Ref< PPContextWrapper > PPContextWrapper::createRenderContext(pp::Instance* instance)
{
    const int32_t attribs[] =
	{
		//PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
		//PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 8,
		PP_GRAPHICS3DATTRIB_WIDTH, c_defaultWidth,
		PP_GRAPHICS3DATTRIB_HEIGHT, c_defaultHeight,
		PP_GRAPHICS3DATTRIB_NONE
    };

	Ref< PPContextWrapper > wrapper = new PPContextWrapper();
	wrapper->m_context = pp::Graphics3D(instance, attribs);
	if (wrapper->m_context.is_null())
	{
		log::error << L"Failed to create render pp::Graphics3D context" << Endl;
		return 0;
	}

	instance->BindGraphics(wrapper->m_context);

	// Create fake primary color and depth; we need to be able to share depth buffer
	// with multiple other render target sets.

	glSetCurrentContextPPAPI(wrapper->m_context.pp_resource());

	// Create frame buffer object.
	T_OGL_SAFE(glGenFramebuffers(1, &wrapper->m_frameBuffer));
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, wrapper->m_frameBuffer));

	// Create color texture.
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0));
	T_OGL_SAFE(glGenTextures(1, &wrapper->m_renderBuffer));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, wrapper->m_renderBuffer));

	T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

	T_OGL_SAFE(glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		c_defaultWidth,
		c_defaultHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		NULL
	));

	// Create depth/stencil buffer.
	T_OGL_SAFE(glGenRenderbuffers(1, &wrapper->m_depthBuffer));
	T_OGL_SAFE(glBindRenderbuffer(GL_RENDERBUFFER, wrapper->m_depthBuffer));
	T_OGL_SAFE(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, c_defaultWidth, c_defaultHeight));

	// Attach buffers to FBO.
	T_OGL_SAFE(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, wrapper->m_renderBuffer, 0));
	T_OGL_SAFE(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, wrapper->m_depthBuffer));
	T_OGL_SAFE(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, wrapper->m_depthBuffer));

	return wrapper;
}

PPContextWrapper::~PPContextWrapper()
{
}

bool PPContextWrapper::resize(int32_t width, int32_t height)
{
	if (width != m_width || height != m_height)
	{
		//m_context.ResizeBuffers(width, height);
		//m_width = width;
		//m_height = height;
	}
	return true;
}

int32_t PPContextWrapper::getWidth() const
{
	return m_width;
}

int32_t PPContextWrapper::getHeight() const
{
	return m_height;
}

bool PPContextWrapper::makeCurrent()
{
	glSetCurrentContextPPAPI(m_context.pp_resource());
	return true;
}

void PPContextWrapper::swapBuffers()
{
	// "Blit" fake primary to real primary.
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_frameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebufferEXT(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	// Issue real flip.
	m_swap.reset();
	pp::MessageLoop::GetForMainThread().PostWork(pp::CompletionCallback(&deferredCallback, this));
	m_swap.wait();
}

void PPContextWrapper::_doSwapBuffers()
{
	m_context.SwapBuffers(pp::CompletionCallback(&flushCallback, this));
}

void PPContextWrapper::_doSwapBuffersFinished()
{
	m_swap.set();
}

PPContextWrapper::PPContextWrapper()
:	m_width(c_defaultWidth)
,	m_height(c_defaultHeight)
,	m_frameBuffer(0)
,	m_renderBuffer(0)
,	m_depthBuffer(0)
{
}

	}
}
