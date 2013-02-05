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
	bool waitVBlank;
};
		
		}

void* cglwCreateContext(void* nativeWindowHandle, void* sharedContext, int depthBits, int stencilBits, int multisample)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	NSOpenGLPixelFormatAttribute attribs[] =
	{
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
		nil
	};
	
	NSOpenGLPixelFormat* nspf = [[NSOpenGLPixelFormat alloc] initWithAttributes: attribs];
	
	WrContext* shwrc = (WrContext*)sharedContext;
	NSOpenGLContext* nsshctx = shwrc ? shwrc->context : 0;
	NSOpenGLContext* nsctx = [[NSOpenGLContext alloc] initWithFormat: nspf shareContext: nsshctx];
	
	WrContext* wrc = new WrContext();
	wrc->view = (NSView*)nativeWindowHandle;
	wrc->context = nsctx;
	wrc->waitVBlank = false;

	[pool release];	

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

void cglwGetSize(void* context, int32_t& outWidth, int32_t& outHeight)
{
	WrContext* wrc = (WrContext*)context;
	if (wrc && wrc->view)
	{
		NSRect bounds = [wrc->view bounds];
		outWidth = int32_t(bounds.size.width);
		outHeight = int32_t(bounds.size.height);
	}
	else
	{
		outWidth =
		outHeight = 0;
	}
}

bool cglwMakeCurrent(void* context)
{
	WrContext* wrc = (WrContext*)context;
	if (wrc)
		[wrc->context makeCurrentContext];
	else
		[NSOpenGLContext clearCurrentContext];

	return true;
}

void cglwSwapBuffers(void* context, bool waitVBlank)
{
	WrContext* wrc = (WrContext*)context;
	if (wrc)
	{
		if (waitVBlank != wrc->waitVBlank)
		{
			int32_t param = waitVBlank ? 1 : 0;
			[wrc->context setValues: &param forParameter: NSOpenGLCPSwapInterval];
			wrc->waitVBlank = waitVBlank;
		}
		[wrc->context flushBuffer];
	}
}

bool cglwCheckHardwarePath()
{
	GLint fragmentGPUProcessing, vertexGPUProcessing;
	CGLGetParameter(CGLGetCurrentContext(), kCGLCPGPUFragmentProcessing, &fragmentGPUProcessing);
	CGLGetParameter(CGLGetCurrentContext(), kCGLCPGPUVertexProcessing, &vertexGPUProcessing);
	return bool(fragmentGPUProcessing && vertexGPUProcessing);
}
	
	}
}
