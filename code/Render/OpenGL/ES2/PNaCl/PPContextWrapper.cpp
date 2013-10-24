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

Ref< PPContextWrapper > PPContextWrapper::createResourceContext(pp::Instance* instance)
{
	T_TRACE(glInitializePPAPI(pp::Module::Get()->get_browser_interface()));

    const int32_t attribs[] =
	{
		PP_GRAPHICS3DATTRIB_NONE
    };

	Ref< PPContextWrapper > wrapper = new PPContextWrapper();
	wrapper->m_context = pp::Graphics3D(instance, attribs);
	if (wrapper->m_context.is_null())
	{
		log::error << L"Failed to create resource-only pp::Graphics3D context" << Endl;
		return 0;
	}

	return wrapper;
}

Ref< PPContextWrapper > PPContextWrapper::createRenderContext(pp::Instance* instance, PPContextWrapper* shareContext)
{
    const int32_t attribs[] =
	{
		PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
		PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 8,
		PP_GRAPHICS3DATTRIB_WIDTH, 320,
		PP_GRAPHICS3DATTRIB_HEIGHT, 200,
		PP_GRAPHICS3DATTRIB_NONE
    };

	Ref< PPContextWrapper > wrapper = new PPContextWrapper();
	if (shareContext)
		wrapper->m_context = pp::Graphics3D(instance, shareContext->m_context, attribs);
	else
		wrapper->m_context = pp::Graphics3D(instance, attribs);

	if (wrapper->m_context.is_null())
	{
		log::error << L"Failed to create render pp::Graphics3D context" << Endl;
		return 0;
	}

	instance->BindGraphics(wrapper->m_context);
	return wrapper;
}

PPContextWrapper::~PPContextWrapper()
{
}

bool PPContextWrapper::resize(int32_t width, int32_t height)
{
	//m_context.ResizeBuffers(width, height);
	return true;
}

int32_t PPContextWrapper::getWidth() const
{
	//int32_t attr[] = { PP_GRAPHICS3DATTRIB_WIDTH, 0 };
	//T_TRACE(m_context.GetAttribs(attr));
	//T_DEBUG(L"getWidth => " << attr[1]);
	return 320; //attr[1];
}

int32_t PPContextWrapper::getHeight() const
{
	//int32_t attr[] = { PP_GRAPHICS3DATTRIB_HEIGHT, 0 };
	//T_TRACE(m_context.GetAttribs(attr));
	//T_DEBUG(L"getHeight => " << attr[1]);
	return 200; //attr[1];
}

bool PPContextWrapper::makeCurrent()
{
	T_TRACE(glSetCurrentContextPPAPI(m_context.pp_resource()));
	return true;
}

void PPContextWrapper::swapBuffers()
{
	m_swap.reset();
	pp::MessageLoop::GetForMainThread().PostWork(pp::CompletionCallback(&deferredCallback, this));
	m_swap.wait();
}

void PPContextWrapper::_doSwapBuffers()
{
	T_TRACE(m_context.SwapBuffers(pp::CompletionCallback(&flushCallback, this)));
}

void PPContextWrapper::_doSwapBuffersFinished()
{
	T_TRACE(m_swap.set());
}

PPContextWrapper::PPContextWrapper()
{
}

	}
}
