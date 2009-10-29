#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#include "Render/OpenGL/ES2/IPhone/EAGLContextWrapper.h"

namespace traktor
{
	namespace render
	{
	
EAGLContextWrapper::EAGLContextWrapper()
:	m_layer(0)
,	m_context(0)
,	m_frameBuffer(0)
,	m_renderBuffer(0)
,	m_depthRenderBuffer(0)
,	m_wantDepthBuffer(false)
,	m_width(0)
,	m_height(0)
,	m_landscape(false)
{
}

bool EAGLContextWrapper::create(void* nativeHandle, bool wantDepthBuffer)
{
	// Native handle are pointer to UIView object;
	// these UIViews must have CAEAGLLayer as layerClass.
	
	UIView* view = (UIView*)nativeHandle;
	CAEAGLLayer* layer = (CAEAGLLayer*)[view layer];

	layer.opaque = YES;
	layer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
//		[NSNumber numberWithBool:NO],
//		kEAGLDrawablePropertyRetainedBacking,
//		kEAGLColorFormatRGBA8,
		kEAGLColorFormatRGB565,
		kEAGLDrawablePropertyColorFormat,
		nil];
		
	EAGLContext* context = [[EAGLContext alloc] initWithAPI : kEAGLRenderingAPIOpenGLES2];
	
	m_layer = layer;
	m_context = (void*)context;
	m_wantDepthBuffer = wantDepthBuffer;
	
	CGRect bounds = [layer bounds];
	m_width = bounds.size.width;
	m_height = bounds.size.height;
	m_landscape = true;	// @fixme Get from view and reset view transforms.

	setCurrent();
	createFrameBuffer();

	return true;
}

void EAGLContextWrapper::destroy()
{
	destroyFrameBuffer();
}

void EAGLContextWrapper::setCurrent()
{
	EAGLContext* context = (EAGLContext*)m_context;
	[EAGLContext setCurrentContext:context];
}

void EAGLContextWrapper::swapBuffers()
{
	EAGLContext* context = (EAGLContext*)m_context;
	glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
	[context presentRenderbuffer:GL_RENDERBUFFER];
}

void EAGLContextWrapper::resize(GLint width, GLint height)
{
	setCurrent();
	destroyFrameBuffer();
	m_width = width;
	m_height = height;
	createFrameBuffer();
}

void EAGLContextWrapper::createFrameBuffer()
{
	EAGLContext* context = (EAGLContext*)m_context;

	glGenFramebuffers(1, &m_frameBuffer);
	glGenRenderbuffers(1, &m_renderBuffer);
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
	[context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)m_layer];
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderBuffer);
	
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &m_width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &m_height);
	
	if (m_wantDepthBuffer)
	{
		glGenRenderbuffers(1, &m_depthRenderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);
	}
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
	}

void EAGLContextWrapper::destroyFrameBuffer()
{
	glDeleteFramebuffers(1, &m_frameBuffer);
	m_frameBuffer = 0;
	
	glDeleteRenderbuffers(1, &m_renderBuffer);
	m_renderBuffer = 0;
	
	if (m_depthRenderBuffer)
	{
		glDeleteRenderbuffers(1, &m_depthRenderBuffer);
		m_depthRenderBuffer = 0;
	}
}

	}
}

