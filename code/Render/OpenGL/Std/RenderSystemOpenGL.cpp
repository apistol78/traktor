#include <algorithm>
#include <locale>
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/Std/Platform.h"
#include "Render/OpenGL/Std/CubeTextureOpenGL.h"
#include "Render/OpenGL/Std/IndexBufferIBO.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"
#include "Render/OpenGL/Std/RenderContextOpenGL.h"
#include "Render/OpenGL/Std/RenderSystemOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetSetOpenGL.h"
#include "Render/OpenGL/Std/RenderViewOpenGL.h"
#include "Render/OpenGL/Std/SimpleTextureOpenGL.h"
#include "Render/OpenGL/Std/StructBufferOpenGL.h"
#include "Render/OpenGL/Std/TimeQueryOpenGL.h"
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
,	m_useProgramCache(true)
#elif defined(__LINUX__)
:	m_display(0)
,	m_screenResources(nullptr)
,	m_useProgramCache(true)
#else
:	m_useProgramCache(true)
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
	T_ASSERT(hDummyRC);

	wglMakeCurrent(hSharedDC, hDummyRC);

	// Initialize GL extensions.
	if (glewInit() != GLEW_OK)
		return false;

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hDummyRC);

	// Finally create new type of context.
	const GLint attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 2,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
		0, 0
	};

	HGLRC hSharedRC = wglCreateContextAttribsARB(hSharedDC, NULL, attribs);
	T_ASSERT(hSharedRC);

	m_resourceContext = new ResourceContextOpenGL(*m_windowShared, hSharedDC, hSharedRC);
	m_resourceContext->enter();

#elif defined(__APPLE__)

	void* resourceContext = cglwCreateContext(0, 0, 0, 0, 0);
	if (!resourceContext)
	{
		log::error << L"Unable to create OpenGL renderer; Unable to create OpenGL 3.2 context" << Endl;
		return false;
	}

	m_resourceContext = new ResourceContextOpenGL(resourceContext);
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

	static int contextAttribs[] =
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 2,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

	GLXContext context = glXCreateContextAttribsARB(m_display, fbc[0], NULL, true, contextAttribs);
	if (!context)
	{
		log::error << L"Unable to create OpenGL renderer; glXCreateContextAttribsARB failed" << Endl;
		return false;
	}

	m_resourceContext = new ResourceContextOpenGL(m_windowShared->getDisplay(), m_windowShared->getWindow(), context);
	m_resourceContext->enter();

	if (glewInit() != GLEW_OK)
		return false;

	m_screenResources = XRRGetScreenResources(m_display, RootWindow(m_display, screen));

#endif

	log::info << L"OpenGL renderer created." << Endl;

	// Log shading version.
	log::info << L"GL_VENDOR = \"" << mbstows((const char*)glGetString(GL_VENDOR)) << L"\"" << Endl;
	log::info << L"GL_RENDERER = \"" << mbstows((const char*)glGetString(GL_RENDERER)) << L"\"" << Endl;
	log::info << L"GL_VERSION = \"" << mbstows((const char*)glGetString(GL_VERSION)) << L"\"" << Endl;
	log::info << L"GL_SHADING_LANGUAGE_VERSION = \"" << mbstows((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)) << L"\"" << Endl;

	// Determine hardware vendor.
	std::wstring vendor = mbstows((const char*)glGetString(GL_VENDOR));
	if (toLower(vendor).find(L"nvidia") != vendor.npos)
		m_info.vendor = AvtNVidia;
	else if (toLower(vendor).find(L"ati") != vendor.npos || toLower(vendor).find(L"amd") != vendor.npos)
		m_info.vendor = AvtAMD;
	else if (toLower(vendor).find(L"intel") != vendor.npos)
		m_info.vendor = AvtIntel;

	// Get GPU memory information; used to determine if or not to preload resources during startup.
	// \fixme Currently only NV HW on Windows and Linux is supported.
#if defined(_WIN32) || defined(__LINUX__)
	if (m_info.vendor == AvtNVidia)
	{
		GLint param = 0;

		glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &param);
		m_info.dedicatedMemoryTotal = param * 1024;

		glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &param);
		m_info.dedicatedMemoryAvailable = param * 1024;

		glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &param);
		m_info.sharedMemoryTotal = param * 1024;
		m_info.sharedMemoryAvailable = param * 1024;

		log::info << L"GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX = " << m_info.dedicatedMemoryTotal << Endl;
		log::info << L"GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX = " << m_info.dedicatedMemoryAvailable << Endl;
		log::info << L"GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX = " << m_info.sharedMemoryTotal << Endl;

		glGetError();
	}
#endif

	m_resourceContext->leave();
	m_resourceContext->setMaxAnisotropy((GLfloat)desc.maxAnisotropy);

	m_useProgramCache = desc.useProgramCache;
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
		m_resourceContext = nullptr;
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
	if (m_screenResources)
	{
		XRRFreeScreenResources(m_screenResources);
		m_screenResources = nullptr;
	}
#endif
}

bool RenderSystemOpenGL::reset(const RenderSystemDesc& desc)
{
	m_resourceContext->setMaxAnisotropy((GLfloat)desc.maxAnisotropy);
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
	if (m_screenResources)
		return m_screenResources->nmode;
	else
		return 0;
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
	if (m_screenResources != nullptr && index < m_screenResources->nmode)
	{
		const auto& m = m_screenResources->modes[index];

		dm.width = m.width;
		dm.height = m.height;
		dm.refreshRate = 0;
		dm.colorBits = 32;

		if (m.hTotal && m.vTotal)
			dm.refreshRate = (int)((double)m.dotClock / ((double)m.hTotal * (double)m.vTotal));
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
	XRRScreenConfiguration* xrrc = XRRGetScreenInfo(m_display, RootWindow(m_display, screen));
	if (xrrc)
	{
		Rotation rotation;
		SizeID sizeId = XRRConfigCurrentConfiguration(xrrc, &rotation);

		int sizes;
		XRRScreenSize* xrrss = XRRConfigSizes(xrrc, &sizes);

		if (sizeId < sizes)
		{
			DisplayMode dm;
			dm.width = xrrss[sizeId].width;
			dm.height = xrrss[sizeId].height;
			dm.refreshRate = 60;
			dm.colorBits = 32;
			return dm;
		}
	}

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
		m_window->setFullScreenStyle();
	else
		m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);

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

	Ref< RenderContextOpenGL > context = new RenderContextOpenGL(m_resourceContext, *m_window, hDC, hRC);
	context->enter();

	if (glewInit() != GLEW_OK)
		return 0;

	context->leave();

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, m_window, context, m_resourceContext);
	if (renderView->reset(desc))
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
	T_ASSERT(viewHandle);

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

	Ref< RenderContextOpenGL > context = new RenderContextOpenGL(m_resourceContext, glcontext);

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, m_windowHandle, context, m_resourceContext);
	if (renderView->reset(desc))
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
		m_window->setFullScreenStyle();
	else
	{
		m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);
		m_window->center();
	}

	int attribs[] = { GLX_RGBA, None };
	XVisualInfo* visual = glXChooseVisual(m_display, DefaultScreen(m_display), attribs);
	if (!visual)
		return 0;

	GLXContext glcontext = glXCreateContext(m_display, visual, m_resourceContext->getGLXContext(), GL_TRUE);
	if (!glcontext)
		return 0;

	Ref< RenderContextOpenGL > context = new RenderContextOpenGL(m_resourceContext, m_window->getDisplay(), m_window->getWindow(), glcontext);
	context->enter();

	if (glewInit() != GLEW_OK)
		return 0;

	context->leave();

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, m_window, context, m_resourceContext);
	if (renderView->reset(desc))
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

	HDC hDC = GetDC((HWND)desc.syswin.hWnd);
	if (!hDC)
		return nullptr;

	int pixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (!pixelFormat)
		return nullptr;

	if (!SetPixelFormat(hDC, pixelFormat, &pfd))
		return nullptr;

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
		return nullptr;
	}

	Ref< RenderContextOpenGL > context = new RenderContextOpenGL(m_resourceContext, (HWND)desc.syswin.hWnd, hDC, hRC);
	context->enter();

	if (glewInit() != GLEW_OK)
		return nullptr;

	context->leave();

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, 0, context, m_resourceContext);
	if (renderView->reset(16, 16))
		return renderView;

	context->destroy();
	context = nullptr;

#elif defined(__APPLE__)

	void* glcontext = cglwCreateContext(
		desc.syswin.view,
		m_resourceContext->getGLContext(),
		desc.depthBits,
		desc.stencilBits,
		0
	);
	if (!glcontext)
		return nullptr;

	Ref< RenderContextOpenGL > context = new RenderContextOpenGL(m_resourceContext, glcontext);

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, 0, context, m_resourceContext);
	if (renderView->reset(16, 16))
		return renderView;

	context->destroy();
	context = nullptr;

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
		return nullptr;
	}

	XVisualInfo* vi = glXGetVisualFromFBConfig(m_display, fbc[0]);
	if (!vi)
	{
		log::error << L"Unable to create OpenGL renderer; No visual information" << Endl;
		return nullptr;
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
		return nullptr;
	}

	Ref< RenderContextOpenGL > context = new RenderContextOpenGL(m_resourceContext, m_display, (::Window)desc.syswin.window, glcontext);
	context->enter();

	if (glewInit() != GLEW_OK)
		return nullptr;

	context->leave();

	Ref< RenderViewOpenGL > renderView = new RenderViewOpenGL(desc, 0, context, m_resourceContext);
	if (renderView->reset(16, 16))
		return renderView;

	context->destroy();
	context = nullptr;

#endif

	return nullptr;
}

Ref< VertexBuffer > RenderSystemOpenGL::createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);

	if (dynamic)
		return new VertexBufferDynamicVBO(m_resourceContext, vertexElements, bufferSize);
	else
		return new VertexBufferStaticVBO(m_resourceContext, vertexElements, bufferSize);
}

Ref< IndexBuffer > RenderSystemOpenGL::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);
	return new IndexBufferIBO(m_resourceContext, indexType, bufferSize, dynamic);
}

Ref< StructBuffer > RenderSystemOpenGL::createStructBuffer(const AlignedVector< StructElement >& structElements, uint32_t bufferSize)
{
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);
#if !defined(__APPLE__)
	return new StructBufferOpenGL(m_resourceContext, structElements, bufferSize);
#else
	return nullptr;
#endif
}

Ref< ISimpleTexture > RenderSystemOpenGL::createSimpleTexture(const SimpleTextureCreateDesc& desc, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);

	Ref< SimpleTextureOpenGL > texture = new SimpleTextureOpenGL(m_resourceContext);
	if (!texture->create(desc))
		return nullptr;

	return texture;
}

Ref< ICubeTexture > RenderSystemOpenGL::createCubeTexture(const CubeTextureCreateDesc& desc, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);

	Ref< CubeTextureOpenGL > texture = new CubeTextureOpenGL(m_resourceContext);
	if (!texture->create(desc))
		return nullptr;

	return texture;
}

Ref< IVolumeTexture > RenderSystemOpenGL::createVolumeTexture(const VolumeTextureCreateDesc& desc, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);

	Ref< VolumeTextureOpenGL > texture = new VolumeTextureOpenGL(m_resourceContext);
	if (!texture->create(desc))
		return nullptr;

	return texture;
}

Ref< IRenderTargetSet > RenderSystemOpenGL::createRenderTargetSet(const RenderTargetSetCreateDesc& desc, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);

	Ref< RenderTargetSetOpenGL > renderTargetSet = new RenderTargetSetOpenGL(m_resourceContext);
	if (!renderTargetSet->create(desc))
		return nullptr;

	return renderTargetSet;
}

Ref< IProgram > RenderSystemOpenGL::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);
	return ProgramOpenGL::create(m_resourceContext, programResource, m_useProgramCache);
}

Ref< ITimeQuery > RenderSystemOpenGL::createTimeQuery() const
{
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);
	return TimeQueryOpenGL::create();
}

void RenderSystemOpenGL::purge()
{
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);
	m_resourceContext->deleteResources();
}

void RenderSystemOpenGL::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

	}
}
