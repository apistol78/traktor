#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>
#include "Render/OpenGL/Std/OsX/CGLWrapper.h"

namespace traktor
{
	namespace render
	{
		namespace
		{
		
struct WrContext
{
	NSView* view;
	NSOpenGLContext* context;
};
		
		}

void* cglwCreateContext(void* nativeWindowHandle, void* sharedContext, int depthBits, int stencilBits, int multisample)
{	
	NSOpenGLPixelFormatAttribute attribs[] =
	{
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFAWindow,
		NSOpenGLPFADepthSize, depthBits,
		NSOpenGLPFAStencilSize, stencilBits,
		NSOpenGLPFASampleBuffers, multisample > 0 ? 1 : 0,
		NSOpenGLPFASamples, multisample,
		nil
	};
	
	NSOpenGLPixelFormat* nspf = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];
	
	WrContext* shwrc = (WrContext*)sharedContext;
	NSOpenGLContext* nsshctx = shwrc ? shwrc->context : 0;
	NSOpenGLContext* nsctx = [[NSOpenGLContext alloc] initWithFormat: nspf shareContext: nsshctx];
	
	WrContext* wrc = new WrContext();
	wrc->view = (NSView*)nativeWindowHandle;
	wrc->context = nsctx;
		
	return (void*)wrc;
}

void cglwDestroyContext(void* context)
{
	WrContext* wrc = (WrContext*)context;
	[wrc->context release];
	delete wrc;
}

void cglwUpdate(void* context)
{
	WrContext* wrc = (WrContext*)context;
	if (wrc)
	{
		if (wrc->view)
			[wrc->context setView: wrc->view];
		[wrc->context update];
	}
}

bool cglwMakeCurrent(void* context)
{
	WrContext* wrc = (WrContext*)context;
	if (wrc)
	{
		[wrc->context makeCurrentContext];
		if (wrc->view)
		{
			NSRect frame = [wrc->view frame];
			glViewport(0, 0, frame.size.width, frame.size.height);
		}
	}
	return true;
}

void cglwSwapBuffers(void* context)
{
	WrContext* wrc = (WrContext*)context;
	if (wrc)
		[wrc->context flushBuffer];
}
	
	}
}
