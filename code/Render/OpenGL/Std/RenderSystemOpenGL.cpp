#include <algorithm>
#include <locale>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/Std/CubeTextureOpenGL.h"
#include "Render/OpenGL/Std/IndexBufferIAR.h"
#include "Render/OpenGL/Std/IndexBufferIBO.h"
#include "Render/OpenGL/Std/ProgramCompilerOpenGL.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"
#include "Render/OpenGL/Std/RenderSystemOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetSetOpenGL.h"
#include "Render/OpenGL/Std/RenderViewOpenGL.h"
#include "Render/OpenGL/Std/SimpleTextureOpenGL.h"
#include "Render/OpenGL/Std/VertexBufferVAR.h"
#include "Render/OpenGL/Std/VertexBufferDynamicVBO.h"
#include "Render/OpenGL/Std/VertexBufferStaticVBO.h"
#include "Render/OpenGL/Std/VolumeTextureOpenGL.h"

#if defined(_WIN32)
#	include "Render/OpenGL/Std/Win32/Window.h"
#elif defined(__APPLE__)
#	include "Render/OpenGL/Std/OsX/CGLWrapper.h"
#	include "Render/OpenGL/Std/OsX/CGLWindow.h"
#endif

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemOpenGL", 0, RenderSystemOpenGL, IRenderSystem)

RenderSystemOpenGL::RenderSystemOpenGL()
#if defined(__APPLE__)
:	m_windowHandle(0)
,	m_maxAnisotrophy(1.0f)
#elif defined(__LINUX__)
:	m_display(0)
,	m_maxAnisotrophy(1.0f)
#else
:	m_maxAnisotrophy(1.0f)
#endif
{
}

bool RenderSystemOpenGL::create(const RenderSystemDesc& desc)
{
#if defined(_WIN32)

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		0,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	m_windowShared = new Window();
	if (!m_windowShared->create())
		return false;

	HDC hSharedDC = GetDC(*m_windowShared);
	if (!hSharedDC)
		return false;

	int pixelFormat = ChoosePixelFormat(hSharedDC, &pfd);
	if (!pixelFormat)
		return false;

	if (!SetPixelFormat(hSharedDC, pixelFormat, &pfd))
		return false;

	// Create a dummy, old, context to load all extensions.
	HGLRC hDummyRC = wglCreateContext(hSharedDC);
	T_ASSERT (hDummyRC);

	wglMakeCurrent(hSharedDC, hDummyRC);

	// Initialize GL extensions.
	if (glewInit() != GLEW_OK)
		return false;

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hDummyRC);

	// Finally create new type of context.
	const GLint attribs[] =
	{
		//WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		//WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 2,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		0, 0
	};

	HGLRC hSharedRC = wglCreateContextAttribsARB(hSharedDC, NULL, attribs);
	T_ASSERT (hSharedRC);

	m_resourceContext = new ContextOpenGL(0, *m_windowShared, hSharedDC, hSharedRC);
	m_resourceContext->enter();

#elif defined(__APPLE__)

	void* resourceContext = cglwCreateContext(0, 0, 0, 0, 0);
	if (!resourceContext)
	{
		log::error << L"Unable to create OpenGL renderer; Unable to create OpenGL 3.2 context" << Endl;
		return false;
	}

	m_resourceContext = new ContextOpenGL(0, resourceContext);
	m_resourceContext->enter();

#elif defined(__LINUX__)

	m_display = XOpenDisplay(0);
	if (!m_display)
	{
		log::error << L"Unable to create OpenGL renderer; Failed to open X display" << Endl;
		return 0;
	}

	int screen = DefaultScreen(m_display);

	// Create required GLX extensions.
	glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
	glXChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)glXGetProcAddressARB((const GLubyte*)"glXChooseFBConfig");
	glXGetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)glXGetProcAddressARB((const GLubyte*)"glXGetVisualFromFBConfig");

	if (
		!glXCreateContextAttribsARB ||
		!glXChooseFBConfig ||
		!glXGetVisualFromFBConfig
	)
	{
		log::error << L"Unable to create OpenGL renderer; Failed to get GLX extensions" << Endl;
		return 0;
	}

	static int visualAttribs[] =
	{
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		None
	};

	int nfbc = 0;
	GLXFBConfig* fbc = glXChooseFBConfig(m_display, screen, visualAttribs, &nfbc);
	if (!fbc || nfbc <= 0)
	{
		log::error << L"Unable to create OpenGL renderer; No framebuffer configuration" << Endl;
		return false;
	}

	XVisualInfo* vi = glXGetVisualFromFBConfig(m_display, fbc[0]);
	if (!vi)
	{
		log::error << L"Unable to create OpenGL renderer; No visual information" << Endl;
		return false;
	}

	m_windowShared = new Window(m_display);
	if (!m_windowShared->create(16, 16))
	{
		log::error << L"Unable to create OpenGL renderer; Failed to create resource window" << Endl;
		return 0;
	}

	GLXContext oldContext = glXCreateContext(m_display, vi, NULL, GL_TRUE);
	if (!oldContext)
	{
		log::error << L"Unable to create OpenGL renderer; glXCreateContext failed" << Endl;
		return false;
	}

	glXMakeCurrent(m_display, m_windowShared->getWindow(), oldContext);

	if (glewInit() != GLEW_OK)
		return false;

	glXMakeCurrent(m_display, 0, 0);
	glXDestroyContext(m_display, oldContext);

	static int contextAttribs[] =
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		None
	};

	GLXContext context = glXCreateContextAttribsARB(m_display, fbc[0], NULL, true, contextAttribs);
	if (!context)
	{
		log::error << L"Unable to create OpenGL renderer; glXCreateContextAttribsARB failed" << Endl;
		return false;
	}

	m_resourceContext = new ContextOpenGL(0, m_windowShared->getDisplay(), m_windowShared->getWindow(), context);
	m_resourceContext->enter();

#endif

	log::info << L"OpenGL " << mbstows((const char *)glGetString(GL_VERSION)) << L" renderer created." << Endl;

	// Get GPU memory information; used to determine if or not to preload resources during startup.
	// \fixme Currently only NV HW on Windows is supported.
#if defined(_WIN32)
	GLint param = 0;

	glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &param);
	m_info.dedicatedMemoryTotal = param * 1024;

	glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &param);
	m_info.sharedMemoryTotal = param * 1024;
	m_info.sharedMemoryAvailable = param * 1024;

	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &param);
	m_info.dedicatedMemoryAvailable = param * 1024;

	glGetError();
#endif

	m_resourceContext->leave();
	m_maxAnisotrophy = (GLfloat)desc.maxAnisotropy;
	return true;
}

void RenderSystemOpenGL::destroy()
{
	if (m_resourceContext)
	{
		// Clean pending resources; don't want to leak resources.
		m_resourceContext->enter();
		m_resourceContext->deleteResources();
		m_resourceContext->leave();

		// Destroy resource context.
		m_resourceContext->destroy();
		m_resourceContext = 0;
	}

#if defined(_WIN32)

	m_windowShared = 0;

#elif defined(__APPLE__)

	if (m_windowHandle)
	{
		cglwDestroyWindow(m_windowHandle);
		m_windowHandle = 0;
	}

#elif defined(__LINUX__)

	m_windowShared = 0;

#endif
}

bool RenderSystemOpenGL::reset(const RenderSystemDesc& desc)
{
	m_maxAnisotrophy = (GLfloat)desc.maxAnisotropy;
	return true;
}

void RenderSystemOpenGL::getInformation(RenderSystemInformation& outInfo) const
{
	outInfo = m_info;
}

uint32_t RenderSystemOpenGL::getDisplayModeCount() const
{
#if defined(_WIN32)

	uint32_t count = 0;

	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	while (EnumDisplaySettings(NULL, count, &dmgl))
		++count;

	return count;

#elif defined(__APPLE__)

	return cglwGetDisplayModeCount();

#else

	int screen = DefaultScreen(m_display);
	int sizes = 0;

	XRRScreenConfiguration* xrrc = XRRGetScreenInfo(m_display, RootWindow(m_display, screen));
	if (xrrc)
	{
		XRRConfigSizes(xrrc, &sizes);
		XRRFreeScreenConfigInfo(xrrc);
	}

	return sizes >= 0 ? uint32_t(sizes) : 0;

#endif
}

DisplayMode RenderSystemOpenGL::getDisplayMode(uint32_t index) const
{
#if defined(_WIN32)

	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	EnumDisplaySettings(NULL, index, &dmgl);

	DisplayMode dm;
	dm.width = dmgl.dmPelsWidth;
	dm.height = dmgl.dmPelsHeight;
	dm.refreshRate = (uint16_t)dmgl.dmDisplayFrequency;
	dm.colorBits = (uint16_t)dmgl.dmBitsPerPel;
	return dm;

#elif defined(__APPLE__)

	DisplayMode dm;
	cglwGetDisplayMode(index, dm);
	return dm;

#else

	DisplayMode dm;

	int screen = DefaultScreen(m_display);
	XRRScreenConfiguration* xrrc = XRRGetScreenInfo(m_display, RootWindow(m_display, screen));
	if (xrrc)
	{
		int sizes;
		XRRScreenSize* xrrss = XRRConfigSizes(xrrc, &sizes);

		if (index < sizes)
		{
			dm.width = xrrss[index].width;
			dm.height = xrrss[index].height;
			dm.refreshRate = 60;
			dm.colorBits = 32;
		}

		XRRFreeScreenConfigInfo(xrrc);
	}

	return dm;
#endif
}

DisplayMode RenderSystemOpenGL::getCurrentDisplayMode() const
{
#if defined(_WIN32)

	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmgl);

	DisplayMode dm;
	dm.width = dmgl.dmPelsWidth;
	dm.height = dmgl.dmPelsHeight;
	dm.refreshRate = (uint16_t)dmgl.dmDisplayFrequency;
	dm.colorBits = (uint16_t)dmgl.dmBitsPerPel;
	return dm;

#elif defined(__APPLE__)

	DisplayMode dm;
	cglwGetCurrentDisplayMode(dm);
	return dm;

#elif defined(__LINUX__)

	int screen = DefaultScreen(m_display);

	DisplayMode dm;
	dm.width = DisplayWidth(m_display, screen);
	dm.height = DisplayHeight(m_display, screen);
	dm.refreshRate = 60;
	dm.colorBits = 32;
	return dm;

#else
	return DisplayMode();
#endif
}

float RenderSystemOpenGL::getDisplayAspectRatio() const
{
	return 0.0f;
}

Ref< IRenderView > RenderSystemOpenGL::createRenderView(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32)

	if (m_window)
		return 0;

	m_window = new Window();
	if (!m_window->create())
	{
		log::error << L"createRenderView failed; unable to create window" << Endl;
		return 0;
	}

	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL Renderer");

	if (desc.fullscreen)
		m_window->setFullScreenStyle(desc.displayMode.width, desc.displayMode.height);
	else
		m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);

	if (desc.fullscreen)
	{
		DEVMODE dmgl;
		std::memset(&dmgl, 0, sizeof(dmgl));
		dmgl.dmSize = sizeof(dmgl);

		for (UINT count = 0; EnumDisplaySettings(NULL, count, &dmgl); ++count)
		{
			if (
				dmgl.dmPelsWidth == desc.displayMode.width &&
				dmgl.dmPelsHeight == desc.displayMode.height
			)
			{
				if (desc.displayMode.colorBits != 0 && dmgl.dmBitsPerPel != desc.displayMode.colorBits)
					continue;
				if (desc.displayMode.colorBits == 0 && dmgl.dmBitsPerPel < 24)
					continue;
				if (desc.displayMode.refreshRate != 0 && dmgl.dmDisplayFrequency != desc.displayMode.refreshRate)
					continue;

				if (ChangeDisplaySettings(&dmgl, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				{
					log::error << L"createRenderView failed; unable to change display settings" << Endl;
					return 0;
				}

				break;
			}
		}
	}

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		(BYTE)desc.depthBits,
		(BYTE)desc.stencilBits,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	HDC hDC = GetDC(*m_window);
	if (!hDC)
	{
		log::error << L"createRenderView failed; unable to get device context" << Endl;
		return 0;
	}

	int pixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (!pixelFormat)
	{
		log::error << L"createRenderView failed; unable to choose pixel format" << Endl;
		return 0;
	}

	if (!SetPixelFormat(hDC, pixelFormat, &pfd))
	{
		log::error << L"createRenderView failed; unable to set pixel format" << Endl;
		return 0;
	}

	const GLint attribs[] =
	{
		//WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		//WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 2,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		0, 0
	};

	HGLRC hRC = wglCreateContextAttribsARB(hDC, m_resourceContext->getGLRC(), attribs);
	if (!hRC)
	{
		log::error << L"createRenderView failed; unable to create WGL context" << Endl;
		return 0;
	}

	Ref< ContextOpenGL > context = new ContextOpenGL(m_resourceContext, *m_window, hDC, hRC);
	context->enter();

	if (glewInit() != GLEW_OK)
		return 0;

	context->leave();

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, m_window, context, m_resourceContext);
	if (renderView->createPrimaryTarget())
		return renderView;

	context->destroy();
	context = 0;

#elif defined(__APPLE__)

	m_windowHandle = cglwCreateWindow(
		!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL Renderer",
		desc.displayMode,
		desc.fullscreen
	);
	if (!m_windowHandle)
	{
		log::error << L"createRenderView failed; unable to create window" << Endl;
		return 0;
	}

	void* viewHandle = cglwGetWindowView(m_windowHandle);
	T_ASSERT (viewHandle);

	void* glcontext = cglwCreateContext(
		viewHandle,
		m_resourceContext->getGLContext(),
		desc.depthBits,
		desc.stencilBits,
		0
	);
	if (!glcontext)
	{
		log::error << L"createRenderView failed; unable to create GL context" << Endl;
		return 0;
	}

	Ref< ContextOpenGL > context = new ContextOpenGL(m_resourceContext, glcontext);

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, m_windowHandle, context, m_resourceContext);
	if (renderView->createPrimaryTarget())
		return renderView;

	log::error << L"createRenderView failed; unable to create primary render target" << Endl;

	context->destroy();
	context = 0;

#elif defined(__LINUX__)

    if (m_window)
        return 0;

	m_window = new Window(m_display);
	if (!m_window->create(desc.displayMode.width, desc.displayMode.height))
	{
		log::error << L"createRenderView failed; unable to create window" << Endl;
		return 0;
	}

	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - OpenGL Renderer");
	m_window->show();

	if (desc.fullscreen)
		m_window->setFullScreenStyle(desc.displayMode.width, desc.displayMode.height);
	else
		m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);

	m_window->center();

	int attribs[] = { GLX_RGBA, None };
	XVisualInfo* visual = glXChooseVisual(m_display, DefaultScreen(m_display), attribs);
	if (!visual)
		return 0;

	GLXContext glcontext = glXCreateContext(m_display, visual, m_resourceContext->getGLXContext(), GL_TRUE);
	if (!glcontext)
		return 0;

	Ref< ContextOpenGL > context = new ContextOpenGL(m_resourceContext, m_window->getDisplay(), m_window->getWindow(), glcontext);
	context->enter();

	if (glewInit() != GLEW_OK)
		return 0;

	context->leave();

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, m_window, context, m_resourceContext);
	if (renderView->createPrimaryTarget())
		return renderView;

	context->destroy();
	context = 0;

#endif

	return 0;
}

Ref< IRenderView > RenderSystemOpenGL::createRenderView(const RenderViewEmbeddedDesc& desc)
{
#if defined(_WIN32)

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		(BYTE)desc.depthBits,
		(BYTE)desc.stencilBits,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	HDC hDC = GetDC((HWND)desc.nativeWindowHandle);
	if (!hDC)
		return 0;

	int pixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (!pixelFormat)
		return 0;

	if (!SetPixelFormat(hDC, pixelFormat, &pfd))
		return 0;

	const GLint attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		0, 0
	};

	HGLRC hRC = wglCreateContextAttribsARB(hDC, m_resourceContext->getGLRC(), attribs);
	if (!hRC)
	{
		log::error << L"createRenderView failed; unable to create WGL context" << Endl;
		return 0;
	}

	Ref< ContextOpenGL > context = new ContextOpenGL(m_resourceContext, (HWND)desc.nativeWindowHandle, hDC, hRC);
	context->enter();

	if (glewInit() != GLEW_OK)
		return 0;

	context->leave();

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, 0, context, m_resourceContext);
	if (renderView->createPrimaryTarget())
		return renderView;

	context->destroy();
	context = 0;

#elif defined(__APPLE__)

	void* glcontext = cglwCreateContext(
		desc.nativeWindowHandle,
		m_resourceContext->getGLContext(),
		desc.depthBits,
		desc.stencilBits,
		0
	);
	if (!glcontext)
		return 0;

	Ref< ContextOpenGL > context = new ContextOpenGL(m_resourceContext, glcontext);

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, 0, context, m_resourceContext);
	if (renderView->createPrimaryTarget())
		return renderView;

	context->destroy();
	context = 0;

#elif defined(__LINUX__)

	static int visualAttribs[] =
	{
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		None
	};

	int nfbc = 0;
	GLXFBConfig* fbc = glXChooseFBConfig(m_display, DefaultScreen(m_display), visualAttribs, &nfbc);
	if (!fbc || nfbc <= 0)
	{
		log::error << L"Unable to create OpenGL renderer; No framebuffer configuration" << Endl;
		return false;
	}

	XVisualInfo* vi = glXGetVisualFromFBConfig(m_display, fbc[0]);
	if (!vi)
	{
		log::error << L"Unable to create OpenGL renderer; No visual information" << Endl;
		return false;
	}

	static int contextAttribs[] =
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		None
	};

	GLXContext glcontext = glXCreateContextAttribsARB(m_display, fbc[0], m_resourceContext->getGLXContext(), True, contextAttribs);
	if (!glcontext)
	{
		log::error << L"Unable to create OpenGL renderer; glXCreateContextAttribsARB failed" << Endl;
		return false;
	}

	Ref< ContextOpenGL > context = new ContextOpenGL(m_resourceContext, m_display, (::Window)desc.nativeWindowHandle, glcontext);
	context->enter();

	if (glewInit() != GLEW_OK)
		return 0;

	context->leave();

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, 0, context, m_resourceContext);
	if (renderView->createPrimaryTarget())
		return renderView;

	context->destroy();
	context = 0;

#endif

	return 0;
}

Ref< VertexBuffer > RenderSystemOpenGL::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	if (dynamic)
		return new VertexBufferDynamicVBO(m_resourceContext, vertexElements, bufferSize);
	else
		return new VertexBufferStaticVBO(m_resourceContext, vertexElements, bufferSize);
}

Ref< IndexBuffer > RenderSystemOpenGL::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);
	return new IndexBufferIBO(m_resourceContext, indexType, bufferSize, dynamic);
}

Ref< ISimpleTexture > RenderSystemOpenGL::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	Ref< SimpleTextureOpenGL > texture = new SimpleTextureOpenGL(m_resourceContext);
	if (!texture->create(desc, m_maxAnisotrophy))
		return 0;

	return texture;
}

Ref< ICubeTexture > RenderSystemOpenGL::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	Ref< CubeTextureOpenGL > texture = new CubeTextureOpenGL(m_resourceContext);
	if (!texture->create(desc))
		return 0;

	return texture;
}

Ref< IVolumeTexture > RenderSystemOpenGL::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	Ref< VolumeTextureOpenGL > texture = new VolumeTextureOpenGL(m_resourceContext);
	if (!texture->create(desc))
		return 0;

	return texture;
}

Ref< RenderTargetSet > RenderSystemOpenGL::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

	Ref< RenderTargetSetOpenGL > renderTargetSet = new RenderTargetSetOpenGL(m_resourceContext);
	if (!renderTargetSet->create(desc))
		return 0;

	return renderTargetSet;
}

Ref< IProgram > RenderSystemOpenGL::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);
	return ProgramOpenGL::create(m_resourceContext, programResource);
}

Ref< IProgramCompiler > RenderSystemOpenGL::createProgramCompiler() const
{
	return new ProgramCompilerOpenGL();
}

Ref< ITimeQuery > RenderSystemOpenGL::createTimeQuery() const
{
	return 0;
}

void RenderSystemOpenGL::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

	}
}
