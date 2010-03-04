#include <algorithm>
#include <locale>
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializable.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/Platform.h"
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
#if defined(T_OPENGL_ES2_HAVE_EGL)
:	m_display(EGL_NO_DISPLAY)
,	m_context(EGL_NO_CONTEXT)
,	m_surface(EGL_NO_SURFACE)
#endif
#if defined(_WIN32)
,	m_hWnd(0)
#endif
{
}

bool RenderSystemOpenGLES2::create(const RenderSystemCreateDesc& desc)
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

	m_globalContext = new ContextOpenGLES2();
	return true;
}

void RenderSystemOpenGLES2::destroy()
{
#if defined(T_OPENGL_ES2_HAVE_EGL)
	eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglDestroyContext(m_display, m_context);
	eglDestroySurface(m_display, m_surface);
	eglTerminate(m_display);

	m_display = EGL_NO_DISPLAY;
	m_context = EGL_NO_CONTEXT;
	m_surface = EGL_NO_SURFACE;
#endif
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
	return DisplayMode();
}

bool RenderSystemOpenGLES2::handleMessages()
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

#endif
	return true;
}

Ref< IRenderView > RenderSystemOpenGLES2::createRenderView(const RenderViewCreateDefaultDesc& desc)
{
#if defined(_WIN32)
	if (m_hWnd)
		return 0;

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

	RenderViewCreateEmbeddedDesc desc2;
	desc2.depthBits = desc.depthBits;
	desc2.stencilBits = desc.stencilBits;
	desc2.multiSample = desc.multiSample;
	desc2.waitVBlank = desc.waitVBlank;
	desc2.nativeWindowHandle = m_hWnd;

	return createRenderView(desc2);
#else
	return 0;
#endif
}

Ref< IRenderView > RenderSystemOpenGLES2::createRenderView(const RenderViewCreateEmbeddedDesc& desc)
{
#if !defined(T_OFFLINE_ONLY)
#	if defined(TARGET_OS_IPHONE)
	EAGLContextWrapper* wrapper = new EAGLContextWrapper();
	if (!wrapper->create(desc.nativeWindowHandle, desc.depthBits != 0))
		return 0;
		
	return new RenderViewOpenGLES2(m_globalContext, wrapper);
#	elif defined(T_OPENGL_ES2_HAVE_EGL)
	const uint32_t c_maxConfigAttrSize = 32;
	const uint32_t c_maxMatchConfigs = 64;

	EGLNativeWindowType nativeWindow = (EGLNativeWindowType)desc.nativeWindowHandle;

	m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (m_display == EGL_NO_DISPLAY) 
	{
		EGLint error = eglGetError();
		log::error << L"Create render view failed; unable to get EGL display (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	if (!eglInitialize(m_display, 0, 0)) 
	{
		EGLint error = eglGetError();
		log::error << L"Create render view failed; unable to initialize EGL (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	EGLint configAttrs[c_maxConfigAttrSize];
	EGLint i = 0;

	configAttrs[i] = EGL_RENDERABLE_TYPE;
	configAttrs[++i] = EGL_OPENGL_ES2_BIT;
	configAttrs[++i] = EGL_DEPTH_SIZE;
	configAttrs[++i] = desc.depthBits;
	configAttrs[++i] = EGL_STENCIL_SIZE;
	configAttrs[++i] = desc.stencilBits;
	configAttrs[++i] = EGL_NONE;

	EGLConfig matchingConfigs[c_maxMatchConfigs];
	EGLint numMatchingConfigs = 0;

	EGLBoolean success = eglChooseConfig(m_display, configAttrs, matchingConfigs, c_maxMatchConfigs, &numMatchingConfigs);
	if (!success)
	{
		EGLint error = eglGetError();
		log::error << L"Create render view failed; unable to create choose EGL config (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	if (numMatchingConfigs == 0)
	{
		EGLint error = eglGetError();
		log::error << L"Create render view failed; no matching configurations" << Endl;
		return 0;
	}

	EGLConfig config = matchingConfigs[0];

	m_surface = eglCreateWindowSurface(m_display, config, nativeWindow, 0);
	if (m_surface == EGL_NO_SURFACE)
	{
		EGLint error = eglGetError();
		log::error << L"Create render view failed; unable to create EGL surface (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	EGLint contextAttrs[] = 
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	m_context = eglCreateContext(m_display, config, EGL_NO_CONTEXT, contextAttrs);
	if (m_context == EGL_NO_CONTEXT)
	{
		EGLint error = eglGetError();
		log::error << L"Create render view failed; unable to create EGL context (" << getEGLErrorString(error) << L")" << Endl;
		return 0;
	}

	if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context))
		return 0;

	return new RenderViewOpenGLES2(m_globalContext, m_display, m_context, m_surface);
#	else
	return 0;
#	endif
#else
	return 0;
#endif
}

Ref< VertexBuffer > RenderSystemOpenGLES2::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	return new VertexBufferOpenGLES2(m_globalContext, vertexElements, bufferSize, dynamic);
}

Ref< IndexBuffer > RenderSystemOpenGLES2::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	return new IndexBufferOpenGLES2(m_globalContext, indexType, bufferSize, dynamic);
}

Ref< ISimpleTexture > RenderSystemOpenGLES2::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureOpenGLES2 > texture = new SimpleTextureOpenGLES2(m_globalContext);
	if (texture->create(desc))
		return texture;
	else
		return texture;
}

Ref< ICubeTexture > RenderSystemOpenGLES2::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	return 0;
}

Ref< IVolumeTexture > RenderSystemOpenGLES2::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	return 0;
}

Ref< RenderTargetSet > RenderSystemOpenGLES2::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
#if !defined(T_OFFLINE_ONLY)
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
	Ref< ProgramOpenGLES2 > program = new ProgramOpenGLES2(m_globalContext);
	if (!program->create(programResource))
		return 0;

	return program;
}

Ref< IProgramCompiler > RenderSystemOpenGLES2::createProgramCompiler() const
{
	return new ProgramCompilerOpenGLES2();
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

	case WM_KEYDOWN:
		if (wParam != VK_ESCAPE)
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
