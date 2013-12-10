#include <algorithm>
#include <locale>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/ES2/CubeTextureOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderSystemOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderViewOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramCompilerOpenGLES2.h"
#include "Render/OpenGL/ES2/ProgramOpenGLES2.h"
#include "Render/OpenGL/ES2/IndexBufferOpenGLES2.h"
#include "Render/OpenGL/ES2/VertexBufferOpenGLES2.h"
#include "Render/OpenGL/ES2/SimpleTextureOpenGLES2.h"
#include "Render/OpenGL/ES2/RenderTargetSetOpenGLES2.h"
#include "Render/OpenGL/ES2/ContextOpenGLES2.h"
#if defined(TARGET_OS_IPHONE)
#	include "Render/OpenGL/ES2/IPhone/EAGLContextWrapper.h"
#endif

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemOpenGLES2", 0, RenderSystemOpenGLES2, IRenderSystem)

RenderSystemOpenGLES2::RenderSystemOpenGLES2()
:	m_nativeHandle(0)
#if defined(_WIN32)
,	m_hWnd(0)
#endif
{
}

bool RenderSystemOpenGLES2::create(const RenderSystemDesc& desc)
{
#if defined(_WIN32)
	WNDCLASS wc;

#	if defined(WINCE)
	wc.style = 0;
#	else
	wc.style = CS_OWNDC;
#	endif
	wc.lpfnWndProc = (WNDPROC)wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = (HINSTANCE)GetModuleHandle(NULL);
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = _T("RenderSystemOpenGLES2_FullScreen");

	RegisterClass(&wc);
#endif

#if !defined(T_OFFLINE_ONLY)
	if (!ContextOpenGLES2::initialize())
		return false;

	m_globalContext = ContextOpenGLES2::createResourceContext(desc.nativeHandle);
	if (!m_globalContext)
		return false;
#endif

	m_nativeHandle = desc.nativeHandle;
	return true;
}

void RenderSystemOpenGLES2::destroy()
{
//#if defined(T_OPENGL_ES2_HAVE_EGL)
//	if (m_display == EGL_NO_DISPLAY)
//		return;
//
//	eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//
//	eglDestroyContext(m_display, m_context);
//	eglDestroySurface(m_display, m_surface);
//	eglTerminate(m_display);
//
//	m_display = EGL_NO_DISPLAY;
//	m_context = EGL_NO_CONTEXT;
//	m_surface = EGL_NO_SURFACE;
//#endif
}

bool RenderSystemOpenGLES2::reset(const RenderSystemDesc& desc)
{
	return true;
}

void RenderSystemOpenGLES2::getInformation(RenderSystemInformation& outInfo) const
{
}

uint32_t RenderSystemOpenGLES2::getDisplayModeCount() const
{
	return 0;
}

DisplayMode RenderSystemOpenGLES2::getDisplayMode(uint32_t index) const
{
	return DisplayMode();
}

DisplayMode RenderSystemOpenGLES2::getCurrentDisplayMode() const
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

#else
	return DisplayMode();
#endif
}

float RenderSystemOpenGLES2::getDisplayAspectRatio() const
{
#if defined(TARGET_OS_IPHONE)
	return 480.0f / 320.0f;
#elif defined(TARGET_OS_IPAD)
	return 1024.0f / 768.0f;
#else
	return 0.0f;
#endif
}

Ref< IRenderView > RenderSystemOpenGLES2::createRenderView(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32)
	if (m_hWnd)
		return 0;

	if (desc.fullscreen)
	{
		m_hWnd = CreateWindow(
			_T("RenderSystemOpenGLES2_FullScreen"),
			_T("Traktor 2.0 OpenGL ES 2.0 Renderer"),
			WS_POPUPWINDOW,
			0,
			0,
			0,
			0,
			NULL,
			NULL,
			static_cast< HMODULE >(GetModuleHandle(NULL)),
			this
		);
		if (!m_hWnd)
			return 0;

		SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, desc.displayMode.width, desc.displayMode.height, SWP_NOMOVE);
	}
	else
	{
		RECT rc;
		SetRect(&rc, 0, 0, desc.displayMode.width, desc.displayMode.height);
		AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, false, 0);

		m_hWnd = CreateWindow(
			_T("RenderSystemOpenGLES2_FullScreen"),
			_T("Traktor 2.0 OpenGL ES 2.0 Renderer"),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			rc.right - rc.left,
			rc.bottom - rc.top,
			NULL,
			NULL,
			static_cast< HMODULE >(GetModuleHandle(NULL)),
			this
		);
		if (!m_hWnd)
			return 0;
	}

	ShowWindow(m_hWnd, SW_SHOWNORMAL);
	UpdateWindow(m_hWnd);

	RenderViewEmbeddedDesc desc2;
	desc2.depthBits = desc.depthBits;
	desc2.stencilBits = desc.stencilBits;
	desc2.multiSample = desc.multiSample;
	desc2.waitVBlank = desc.waitVBlank;
	desc2.nativeWindowHandle = m_hWnd;
	desc2.stereoscopic = false;

	return createRenderView(desc2);
#elif defined(__PNACL__) || defined(__EMSCRIPTEN__)
	Ref< ContextOpenGLES2 > context = ContextOpenGLES2::createContext(
		m_globalContext,
		m_nativeHandle,
		0
	);
	return new RenderViewOpenGLES2(m_globalContext, context);
#else
	return 0;
#endif
}

Ref< IRenderView > RenderSystemOpenGLES2::createRenderView(const RenderViewEmbeddedDesc& desc)
{
#if !defined(T_OFFLINE_ONLY)
	Ref< ContextOpenGLES2 > context = ContextOpenGLES2::createContext(
		m_globalContext,
		m_nativeHandle,
		desc.nativeWindowHandle
	);
	return new RenderViewOpenGLES2(m_globalContext, context);
#else
	return 0;
#endif
}

Ref< VertexBuffer > RenderSystemOpenGLES2::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	return new VertexBufferOpenGLES2(m_globalContext, vertexElements, bufferSize, dynamic);
#else
	return 0;
#endif
}

Ref< IndexBuffer > RenderSystemOpenGLES2::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	return new IndexBufferOpenGLES2(m_globalContext, indexType, bufferSize, dynamic);
#else
	return 0;
#endif
}

Ref< ISimpleTexture > RenderSystemOpenGLES2::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	Ref< SimpleTextureOpenGLES2 > texture = new SimpleTextureOpenGLES2(m_globalContext);
	if (texture->create(desc))
		return texture;
	else
		return texture;
#else
	return 0;
#endif
}

Ref< ICubeTexture > RenderSystemOpenGLES2::createCubeTexture(const CubeTextureCreateDesc& desc)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	Ref< CubeTextureOpenGLES2 > texture = new CubeTextureOpenGLES2(m_globalContext);
	if (texture->create(desc))
		return texture;
	else
		return texture;
#else
	return 0;
#endif
}

Ref< IVolumeTexture > RenderSystemOpenGLES2::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	return 0;
}

Ref< RenderTargetSet > RenderSystemOpenGLES2::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	Ref< RenderTargetSetOpenGLES2 > renderTargetSet = new RenderTargetSetOpenGLES2(m_globalContext);
	if (renderTargetSet->create(desc))
		return renderTargetSet;
	else
		return 0;
#else
	return 0;
#endif
}

Ref< IProgram > RenderSystemOpenGLES2::createProgram(const ProgramResource* programResource)
{
#if !defined(T_OFFLINE_ONLY)
	T_ANONYMOUS_VAR(IContext::Scope)(m_globalContext);
	return ProgramOpenGLES2::create(m_globalContext, programResource);
#else
	return 0;
#endif
}

Ref< IProgramCompiler > RenderSystemOpenGLES2::createProgramCompiler() const
{
	return new ProgramCompilerOpenGLES2();
}

void RenderSystemOpenGLES2::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

#if defined(_WIN32)

LRESULT RenderSystemOpenGLES2::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RenderSystemOpenGLES2* renderSystem = reinterpret_cast< RenderSystemOpenGLES2* >(GetWindowLongPtr(hWnd, 0));
	LPCREATESTRUCT createStruct;
	LRESULT result = TRUE;

	switch (uMsg)
	{
	case WM_CREATE:
		createStruct = reinterpret_cast< LPCREATESTRUCT >(lParam);
		renderSystem = reinterpret_cast< RenderSystemOpenGLES2* >(createStruct->lpCreateParams);
		SetWindowLongPtr(hWnd, 0, reinterpret_cast< LONG_PTR >(renderSystem));
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		SetWindowLongPtr(hWnd, 0, 0);
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
