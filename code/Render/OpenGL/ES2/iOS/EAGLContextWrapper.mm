#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#include "Core/Log/Log.h"
#include "Render/OpenGL/ES2/iOS/EAGLContextWrapper.h"

namespace traktor
{
	namespace render
	{
	
EAGLContextWrapper::EAGLContextWrapper()
:	m_layer(0)
,	m_context(0)
,	m_frameBuffer(0)
,	m_renderBuffer(0)
,	m_depthBuffer(0)
,	m_width(0)
,	m_height(0)
{
}

bool EAGLContextWrapper::create()
{
	EAGLContext* context = [[EAGLContext alloc] initWithAPI: kEAGLRenderingAPIOpenGLES2];
	m_context = (void*)context;
	return true;
}

bool EAGLContextWrapper::create(EAGLContextWrapper* shareContext, void* nativeHandle)
{
	// Native handle are pointer to UIView object;
	// these UIViews must have CAEAGLLayer as layerClass.
	
	UIView* view = (UIView*)nativeHandle;
	CAEAGLLayer* layer = (CAEAGLLayer*)[view layer];

	layer.opaque = YES;
	layer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
		[NSNumber numberWithBool:NO],
		kEAGLDrawablePropertyRetainedBacking,
		kEAGLColorFormatRGB565,
		kEAGLDrawablePropertyColorFormat,
		nil];
	
	EAGLContext* shareCtx = (EAGLContext*)shareContext->m_context;
	EAGLSharegroup* shareGroup = [shareCtx sharegroup];
	EAGLContext* context = [[EAGLContext alloc] initWithAPI: kEAGLRenderingAPIOpenGLES2 sharegroup: shareGroup];
	
	m_layer = layer;
	m_context = (void*)context;
	
	CGRect bounds = [layer bounds];
	m_width = bounds.size.width;
	m_height = bounds.size.height;

	setCurrent(this);
	createFrameBuffer();

	return true;
}

void EAGLContextWrapper::destroy()
{
	destroyFrameBuffer();
}

bool EAGLContextWrapper::setCurrent(EAGLContextWrapper* context)
{
	if (context)
	{
		EAGLContext* eaglctx = (EAGLContext*)context->m_context;
		[EAGLContext setCurrentContext:eaglctx];
	}
	else
	{
		[EAGLContext setCurrentContext:nil];
	}
	return true;
}

void EAGLContextWrapper::swapBuffers()
{
	EAGLContext* context = (EAGLContext*)m_context;
	glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
	[context presentRenderbuffer:GL_RENDERBUFFER];
}

void EAGLContextWrapper::resize(GLint width, GLint height)
{
	setCurrent(this);
	destroyFrameBuffer();
	m_width = width;
	m_height = height;
	createFrameBuffer();
}

bool EAGLContextWrapper::getLandscape() const
{
	UIDeviceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
	return UIInterfaceOrientationIsLandscape(orientation); // (orientation == UIDeviceOrientationLandscapeLeft || orientation == UIDeviceOrientationLandscapeRight);
}

void EAGLContextWrapper::createFrameBuffer()
{
	EAGLContext* context = (EAGLContext*)m_context;

	// Create primary buffer.
	glGenFramebuffers(1, &m_frameBuffer);
	glGenRenderbuffers(1, &m_renderBuffer);
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
	
	[context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)m_layer];
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderBuffer);
	
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &m_width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &m_height);
	
	// Create depth buffer.
	glGenRenderbuffers(1, &m_depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuffer);
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
}

void EAGLContextWrapper::destroyFrameBuffer()
{
	glDeleteFramebuffers(1, &m_frameBuffer);
	m_frameBuffer = 0;
	
	glDeleteRenderbuffers(1, &m_renderBuffer);
	m_renderBuffer = 0;
	
	glDeleteRenderbuffers(1, &m_depthBuffer);
	m_depthBuffer = 0;
}

int32_t EAGLContextWrapper::getCurrentWidth()
{
	return [UIScreen mainScreen].bounds.size.width;
}

int32_t EAGLContextWrapper::getCurrentHeight()
{
	return [UIScreen mainScreen].bounds.size.height;
}

	}
}

