#include <algorithm>
#include <locale>
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/RenderSystemOpenGL.h"
#include "Render/OpenGL/Std/RenderViewOpenGL.h"
#include "Render/OpenGL/Std/ProgramCompilerOpenGL.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"
#include "Render/OpenGL/Std/VertexBufferVAR.h"
#include "Render/OpenGL/Std/VertexBufferVBO.h"
#include "Render/OpenGL/Std/IndexBufferIAR.h"
#include "Render/OpenGL/Std/IndexBufferIBO.h"
#include "Render/OpenGL/Std/SimpleTextureOpenGL.h"
#include "Render/OpenGL/Std/CubeTextureOpenGL.h"
#include "Render/OpenGL/Std/VolumeTextureOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetSetOpenGL.h"

#if defined(__APPLE__)
#	include "Render/OpenGL/Std/OsX/CGLWrapper.h"
#	include "Render/OpenGL/Std/OsX/CGLWindow.h"
#endif

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemOpenGL", 0, RenderSystemOpenGL, IRenderSystem)

RenderSystemOpenGL::RenderSystemOpenGL()
#if defined(_WIN32)
:	m_hWndShared(NULL)
,	m_hWnd(NULL)
#elif defined(__APPLE__)
:	m_windowHandle(0)
#endif
,	m_maxAnisotrophy(1.0f)
{
}

bool RenderSystemOpenGL::create(const RenderSystemCreateDesc& desc)
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

	WNDCLASS wc;

	// Create shared context class.
	wc.style         = 0;
	wc.lpfnWndProc   = DefWindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = (HINSTANCE)GetModuleHandle(NULL);
	wc.hIcon         = NULL;
	wc.hCursor       = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = _T("RenderSystemOpenGL_GlobalContext");

	RegisterClass(&wc);

	// Create full screen window class.
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)wndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(this);
	wc.hInstance     = (HINSTANCE)GetModuleHandle(NULL);
	wc.hIcon         = NULL;
	wc.hCursor       = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = _T("RenderSystemOpenGL_FullScreen");

	RegisterClass(&wc);

	// Create window for shared context.
	m_hWndShared = CreateWindow(
		_T("RenderSystemOpenGL_GlobalContext"),
		_T(""),
		0,
		-1,
		-1,
		0,
		0,
		NULL,
		NULL,
		(HINSTANCE)GetModuleHandle(NULL),
		NULL
	);

	HDC hSharedDC = GetDC(m_hWndShared);
	if (!hSharedDC)
		return false;

	int pixelFormat = ChoosePixelFormat(hSharedDC, &pfd);
	if (!pixelFormat)
		return false;

	if (!SetPixelFormat(hSharedDC, pixelFormat, &pfd))
		return false;

	HGLRC hSharedRC = wglCreateContext(hSharedDC);
	T_ASSERT (hSharedRC);

	m_resourceContext = new ContextOpenGL(m_hWndShared, hSharedDC, hSharedRC);
	m_resourceContext->enter();

	if (!opengl_initialize_extensions())
		return false;

	m_resourceContext->leave();

#elif defined(__APPLE__)

	void* resourceContext = cglwCreateContext(0, 0, 0, 0, 0);
	m_resourceContext = new ContextOpenGL(resourceContext);

	m_resourceContext->enter();

	if (!opengl_initialize_extensions())
		return false;

	m_resourceContext->leave();

	m_title = desc.windowTitle ? desc.windowTitle : L"Traktor (OpenGL)";

#else	// LINUX

	/*
	log::info << L"Open X display..." << Endl;

	Display* display = XOpenDisplay(0);
	T_ASSERT (display);

	if (!display)
		log::error << L"Unable to open X display!" << Endl;

	log::info << L"Choose X visual..." << Endl;

	int attribs[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 16, None };
	XVisualInfo* visual = glXChooseVisual(display, DefaultScreen(display), attribs);
	T_ASSERT (visual);

	if (!visual)
		log::error << L"Unable to choose X visual!" << Endl;

	log::info << L"Create GLX context..." << Endl;

	GLXContext resourceContext = glXCreateContext(display, visual, NULL, GL_TRUE);
	T_ASSERT (resourceContext);

	if (!globalContext)
		log::error << L"Unable to create GLX context!" << Endl;

	log::info << L"Create resource context..." << Endl;

	// @hack Need to have some GLXVisual ready, need to create a dummy window.
	m_resourceContext = new ContextOpenGL(display, 0, resourceContext);
	m_resourceContext->enter();

	if (!opengl_initialize_extensions())
		T_BREAKPOINT;
	*/

	m_resourceContext->leave();

#endif

	m_maxAnisotrophy = (GLfloat)desc.maxAnisotropy;
	return true;
}

void RenderSystemOpenGL::destroy()
{
#if defined(__APPLE__)
	if (m_windowHandle)
	{
		cglwDestroyWindow(m_windowHandle);
		m_windowHandle = 0;
	}
#endif

	if (m_resourceContext)
	{
		m_resourceContext->destroy();
		m_resourceContext = 0;
	}
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
	return DisplayMode();
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

#else
	return DisplayMode();
#endif
}

bool RenderSystemOpenGL::handleMessages()
{
#if defined(_WIN32)

	bool going = true;
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret <= 0 || msg.message == WM_QUIT)
			going = false;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return going;

#elif defined(__APPLE__)

	if (!m_windowHandle)
		return false;
		
	return cglwUpdateWindow(m_windowHandle);

#else
	return true;
#endif
}

Ref< IRenderView > RenderSystemOpenGL::createRenderView(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32)

	UINT style = 0;

	if (desc.fullscreen)
		style = WS_POPUPWINDOW;
	else
		style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;

	m_hWnd = CreateWindow(
		_T("RenderSystemOpenGL_FullScreen"),
		_T("Traktor 2.0 OpenGL Renderer"),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		NULL,
		NULL,
		static_cast< HMODULE >(GetModuleHandle(NULL)),
		this
	);
	if (!m_hWnd)
	{
		log::error << L"createRenderView failed; unable to create window" << Endl;
		return 0;
	}

	if (desc.fullscreen)
	{
		SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, desc.displayMode.width, desc.displayMode.height, SWP_SHOWWINDOW);
		ShowWindow(m_hWnd, SW_MAXIMIZE);
	}
	else
	{
		RECT rcWindow;
		GetWindowRect(m_hWnd, &rcWindow);

		RECT rcClient;
		GetClientRect(m_hWnd, &rcClient);

		int32_t windowWidth = rcWindow.right - rcWindow.left;
		int32_t windowHeight = rcWindow.bottom - rcWindow.top;

		int32_t realClientWidth = rcClient.right - rcClient.left;
		int32_t realClientHeight = rcClient.bottom - rcClient.top;

		windowWidth = (windowWidth - realClientWidth) + desc.displayMode.width;
		windowHeight = (windowHeight - realClientHeight) + desc.displayMode.height;

		SetWindowPos(m_hWnd, 0, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);
		ShowWindow(m_hWnd, SW_NORMAL);
	}

	UpdateWindow(m_hWnd);

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

	HDC hDC = GetDC(m_hWnd);
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

	HGLRC hRC = wglCreateContext(hDC);
	if (!hRC)
	{
		log::error << L"createRenderView failed; unable to create WGL context" << Endl;
		return 0;
	}

	Ref< ContextOpenGL > context = new ContextOpenGL(m_hWnd, hDC, hRC);
	m_resourceContext->share(context);

	return new RenderViewOpenGL(desc, context, m_resourceContext, m_hWnd);

#elif defined(__APPLE__)

	m_windowHandle = cglwCreateWindow(
		m_title,
		desc.displayMode,
		desc.fullscreen
	);
	if (!m_windowHandle)
		return 0;
		
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
		return 0;

	Ref< ContextOpenGL > context = new ContextOpenGL(glcontext);

	return new RenderViewOpenGL(desc, context, m_resourceContext, m_windowHandle);

#else
	return 0;
#endif
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

	HGLRC hRC = wglCreateContext(hDC);
	if (!hRC)
		return 0;

	Ref< ContextOpenGL > context = new ContextOpenGL((HWND)desc.nativeWindowHandle, hDC, hRC);
	m_resourceContext->share(context);

	context->enter();

	if (!opengl_initialize_extensions())
		return 0;

	context->leave();

	return new RenderViewOpenGL(desc, context, m_resourceContext, (HWND)desc.nativeWindowHandle);

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

	Ref< ContextOpenGL > context = new ContextOpenGL(glcontext);

	return new RenderViewOpenGL(desc, context, m_resourceContext, 0);

#else	// LINUX

	struct Handle { Display* display; Window window; }* handle = reinterpret_cast< Handle* >(desc.nativeWindowHandle);

	int attribs[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 16, None };
	XVisualInfo* visual = glXChooseVisual(handle->display, DefaultScreen(handle->display), attribs);
	if (!visual)
	{
		log::error << L"Unable to create renderview, glXChooseVisual failed" << Endl;
		return 0;
	}

	// @hack Need to have global context ready.
	GLXContext ctx = glXCreateContext(handle->display, visual, /*m_globalContext->getGLXContext()*/NULL, GL_TRUE);
	if (!ctx)
	{
		log::error << L"Unable to create renderview, glXCreateContext failed" << Endl;
		return 0;
	}

	log::info << L"GLX context created successfully" << Endl;

	Ref< ContextOpenGL > context = new ContextOpenGL(handle->display, handle->window, ctx);

	context->enter();

	log::info << L"Loading OpenGL extensions..." << Endl;

	if (!opengl_initialize_extensions())
	{
		log::error << L"Unable to create renderview, failed to initialize required OpenGL extensions" << Endl;
		return 0;
	}

	log::info << L"OpenGL extensions loaded successfully" << Endl;

	context->leave();

	return new RenderViewOpenGL(desc, context, m_resourceContext);

#endif
}

Ref< VertexBuffer > RenderSystemOpenGL::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

#if defined(_WIN32)
	if (glGenBuffersARB)
		return new VertexBufferVBO(m_resourceContext, vertexElements, bufferSize, dynamic);
	else
		return new VertexBufferVAR(m_resourceContext, vertexElements, bufferSize, dynamic);
#elif defined(__APPLE__)
	if (opengl_have_extension("GL_ARB_vertex_buffer_object"))
		return new VertexBufferVBO(m_resourceContext, vertexElements, bufferSize, dynamic);
	else
		return new VertexBufferVAR(m_resourceContext, vertexElements, bufferSize, dynamic);
#else
	return new VertexBufferVAR(m_resourceContext, vertexElements, bufferSize, dynamic);
#endif
}

Ref< IndexBuffer > RenderSystemOpenGL::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);

#if defined(_WIN32)
	if (glGenBuffersARB)
		return new IndexBufferIBO(m_resourceContext, indexType, bufferSize, dynamic);
	else
		return new IndexBufferIAR(m_resourceContext, indexType, bufferSize);
#elif defined(__APPLE__)
	if (opengl_have_extension("GL_ARB_vertex_buffer_object"))
		return new IndexBufferIBO(m_resourceContext, indexType, bufferSize, dynamic);
	else
		return new IndexBufferIAR(m_resourceContext, indexType, bufferSize);
#else
	return new IndexBufferIAR(m_resourceContext, indexType, bufferSize);
#endif
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

Ref< IProgram > RenderSystemOpenGL::createProgram(const ProgramResource* programResource)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);
	return ProgramOpenGL::create(m_resourceContext, programResource);
}

Ref< IProgramCompiler > RenderSystemOpenGL::createProgramCompiler() const
{
	return new ProgramCompilerOpenGL();
}

#if defined(_WIN32)

LRESULT RenderSystemOpenGL::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RenderSystemOpenGL* renderSystem = reinterpret_cast< RenderSystemOpenGL* >(GetWindowLongPtr(hWnd, 0));
	LPCREATESTRUCT createStruct;
	LRESULT result = TRUE;

	switch (uMsg)
	{
	case WM_CREATE:
		createStruct = reinterpret_cast< LPCREATESTRUCT >(lParam);
		renderSystem = reinterpret_cast< RenderSystemOpenGL* >(createStruct->lpCreateParams);
		SetWindowLongPtr(hWnd, 0, reinterpret_cast< LONG_PTR >(renderSystem));
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_ERASEBKGND:
		break;

	case WM_SETCURSOR:
		SetCursor(NULL);
		break;

	default:
		result = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return result;
}

#endif

	}
}
