#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Graphics/IGraphicsSystem.h"
#include "Render/VertexElement.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Sw/RenderSystemSw.h"
#include "Render/Sw/RenderViewSw.h"
#include "Render/Sw/VertexBufferSw.h"
#include "Render/Sw/IndexBufferSw.h"
#include "Render/Sw/SimpleTextureSw.h"
#include "Render/Sw/RenderTargetSetSw.h"
#include "Render/Sw/ProgramSw.h"
#include "Render/Sw/ProgramCompilerSw.h"
#include "Render/Sw/ProgramResourceSw.h"
#include "Render/Sw/Emitter/Emitter.h"
#include "Render/Sw/Emitter/EmitterContext.h"

#if defined(_WIN32)
#	if !defined(WINCE)
#		include "Graphics/Gdi/GraphicsSystemGdi.h"
#	else
#		include "Graphics/DdWm5/GraphicsSystemDdWm5.h"
#	endif
#endif

#if defined(_WIN32) && !defined(WINCE) && !defined(_WIN64)
#	include "Render/Sw/Core/x86/JitX86.h"
typedef traktor::render::JitX86 ProcessorImpl;
#elif !defined(WINCE)
#	include "Render/Sw/Core/Interpreter.h"
typedef traktor::render::Interpreter ProcessorImpl;
#else
#	include "Render/Sw/Core/InterpreterFixed.h"
typedef traktor::render::InterpreterFixed ProcessorImpl;
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

const TCHAR* c_windowClassName = _T("TraktorRenderSystem");

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemSw", 0, RenderSystemSw, IRenderSystem)

RenderSystemSw::RenderSystemSw()
#if defined(_WIN32)
:	m_hWnd(NULL)
#endif
{
	m_processor = new ProcessorImpl();
}

bool RenderSystemSw::create(const RenderSystemCreateDesc& desc)
{
#if defined(_WIN32)

	WNDCLASS wc;
	std::memset(&wc, 0, sizeof(wc));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(this);
	wc.lpfnWndProc = (WNDPROC)wndProc;
	wc.hInstance = static_cast< HINSTANCE >(GetModuleHandle(0));
	wc.hIcon = NULL;
	wc.hCursor = static_cast< HCURSOR >(LoadCursor(NULL, IDC_ARROW));
	wc.lpszClassName = c_windowClassName;
	RegisterClass(&wc);

#endif
	return true;
}

void RenderSystemSw::destroy()
{
}

uint32_t RenderSystemSw::getDisplayModeCount() const
{
	return uint32_t(m_displayModes.size());
}

DisplayMode RenderSystemSw::getDisplayMode(uint32_t index) const
{
	DisplayMode dm;
	dm.width = m_displayModes[index].width;
	dm.height = m_displayModes[index].height;
	dm.refreshRate = 0;
	dm.colorBits = m_displayModes[index].bits;
	return dm;
}

DisplayMode RenderSystemSw::getCurrentDisplayMode() const
{
	return DisplayMode();
}

float RenderSystemSw::getDisplayAspectRatio() const
{
	return 0.0f;
}

IRenderSystem::HandleResult RenderSystemSw::handleMessages()
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

	return going ? HrSuccess : HrTerminate;

#else
	return false;
#endif
}

Ref< IRenderView > RenderSystemSw::createRenderView(const RenderViewDefaultDesc& desc)
{
	Ref< graphics::IGraphicsSystem > graphicsSystem;
	graphics::CreateDesc graphicsDesc;

#if defined(_WIN32)

	DWORD style = 0;

	if (desc.fullscreen)
		style = WS_POPUP;
	else
		style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;

	m_hWnd = CreateWindow(
		c_windowClassName,
		_T("Traktor 2.0 Software Renderer"),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		desc.displayMode.width,
		desc.displayMode.height,
		NULL,
		NULL,
		static_cast< HMODULE >(GetModuleHandle(NULL)),
		this
	);
	if (!m_hWnd)
		return 0;

	RECT rcWindow, rcClient;
	GetWindowRect(m_hWnd, &rcWindow);
	GetClientRect(m_hWnd, &rcClient);

	int32_t windowWidth = rcWindow.right - rcWindow.left;
	int32_t windowHeight = rcWindow.bottom - rcWindow.top;

	int32_t realClientWidth = rcClient.right - rcClient.left;
	int32_t realClientHeight = rcClient.bottom - rcClient.top;

	windowWidth = (windowWidth - realClientWidth) + desc.displayMode.width;
	windowHeight = (windowHeight - realClientHeight) + desc.displayMode.height;

	if (desc.fullscreen)
	{
		SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, windowWidth, windowHeight, SWP_NOSIZE | SWP_NOMOVE);
		ShowWindow(m_hWnd, SW_MAXIMIZE);
	}
	else
	{
		SetWindowPos(m_hWnd, NULL, 0, 0, windowWidth, windowHeight, SWP_NOZORDER | SWP_NOMOVE);
		ShowWindow(m_hWnd, SW_SHOW);
	}

	UpdateWindow(m_hWnd);

#endif

#if defined(_WIN32)
#	if !defined(WINCE)
	graphicsSystem = new graphics::GraphicsSystemGdi();
#	else
	graphicsSystem = new graphics::GraphicsSystemDdWm5();
#	endif
#endif

	graphicsDesc.windowHandle = m_hWnd;
	graphicsDesc.fullScreen = desc.fullscreen;
	graphicsDesc.displayMode.width = desc.displayMode.width;
	graphicsDesc.displayMode.height = desc.displayMode.height;
	graphicsDesc.displayMode.bits = desc.displayMode.colorBits;
	graphicsDesc.pixelFormat = graphics::PfeR5G6B5;

	if (!graphicsSystem->create(graphicsDesc))
		return 0;

	return new RenderViewSw(this, graphicsSystem, m_processor);
}

Ref< IRenderView > RenderSystemSw::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	Ref< graphics::IGraphicsSystem > graphicsSystem;
	graphics::CreateDesc graphicsDesc;

	graphicsDesc.windowHandle = desc.nativeWindowHandle;
	graphicsDesc.fullScreen = false;
	graphicsDesc.displayMode.width = 16;
	graphicsDesc.displayMode.height = 16;
	graphicsDesc.displayMode.bits = 16;
	graphicsDesc.pixelFormat = graphics::PfeR5G6B5;

#if defined(_WIN32)
#	if !defined(WINCE)
	graphicsSystem = new graphics::GraphicsSystemGdi();
#	else
	graphicsSystem = new graphics::GraphicsSystemDdWm5();
#	endif

	RECT rc;
	GetClientRect((HWND)desc.nativeWindowHandle, &rc);
	graphicsDesc.displayMode.width = std::max(int(rc.right - rc.left), 16);
	graphicsDesc.displayMode.height = std::max(int(rc.bottom - rc.top), 16);

#endif

	if (!graphicsSystem->create(graphicsDesc))
		return 0;

	return new RenderViewSw(this, graphicsSystem, m_processor);
}

Ref< VertexBuffer > RenderSystemSw::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	return new VertexBufferSw(vertexElements, bufferSize);
}

Ref< IndexBuffer > RenderSystemSw::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	return new IndexBufferSw(indexType, bufferSize);
}

Ref< ISimpleTexture > RenderSystemSw::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureSw > texture = new SimpleTextureSw();
	if (!texture->create(desc))
		return 0;
	return texture;
}

Ref< ICubeTexture > RenderSystemSw::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	return 0;
}

Ref< IVolumeTexture > RenderSystemSw::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	return 0;
}

Ref< RenderTargetSet > RenderSystemSw::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetSw > renderTargetSet = new RenderTargetSetSw();
	if (!renderTargetSet->create(desc))
		return 0;
	return renderTargetSet;
}

Ref< IProgram > RenderSystemSw::createProgram(const ProgramResource* programResource)
{
	Ref< const ProgramResourceSw > resource = dynamic_type_cast< const ProgramResourceSw* >(programResource);
	if (!resource)
		return 0;

	const ShaderGraph* shaderGraph = resource->getShaderGraph();

	RefArray< VertexOutput > vertexOutputs;
	RefArray< PixelOutput > pixelOutputs;

	shaderGraph->findNodesOf< VertexOutput >(vertexOutputs);
	shaderGraph->findNodesOf< PixelOutput >(pixelOutputs);

	if (vertexOutputs.size() != 1 || pixelOutputs.size() != 1)
	{
		log::error << L"Unable to generate program; incorrect number of outputs" << Endl;
		return 0;
	}

	EmitterContext::Parameters parameters;
	EmitterContext cx(shaderGraph, parameters);

	// Emit outputs.
	cx.emit(pixelOutputs[0]);
	cx.emit(vertexOutputs[0]);

	// Compile programs.
	Processor::image_t vertexProgram = m_processor->compile(cx.getVertexProgram());
	if (!vertexProgram)
		return 0;

	Processor::image_t pixelProgram = m_processor->compile(cx.getPixelProgram());
	if (!pixelProgram)
		return 0;

#if defined(_DEBUG)
	log::info << L"Vertex program:" << Endl;
	cx.getVertexProgram().dump(log::info, parameters.uniforms);
	log::info << Endl;

	log::info << L"Pixel program:" << Endl;
	cx.getPixelProgram().dump(log::info, parameters.uniforms);
	log::info << Endl;
#endif

	// Get emitter parameters.
	std::map< handle_t, std::pair< int, int > > parameterMap;
	for (std::map< std::wstring, Variable* >::const_iterator i = parameters.uniforms.begin(); i != parameters.uniforms.end(); ++i)
		parameterMap[getParameterHandle(i->first)] = std::make_pair(i->second->reg, i->second->size);

	std::map< handle_t, int > samplerMap;
	for (std::map< std::wstring, int >::const_iterator i = parameters.samplers.begin(); i != parameters.samplers.end(); ++i)
		samplerMap[getParameterHandle(i->first)] = i->second;

	return new ProgramSw(
		parameterMap,
		samplerMap,
		vertexProgram,
		pixelProgram,
		cx.getRenderState(),
		cx.getInterpolatorCount()
	);
}

Ref< IProgramCompiler > RenderSystemSw::createProgramCompiler() const
{
	return new ProgramCompilerSw();
}

void RenderSystemSw::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

#if defined(_WIN32)

LRESULT WINAPI RenderSystemSw::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = TRUE;
	
	switch (uMsg)
	{
	case WM_CREATE:
		break;
	
#if defined(WINCE)
	case WM_LBUTTONDOWN:
		DestroyWindow(hWnd);
		break;
#endif

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			DestroyWindow(hWnd);
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
		
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		
	case WM_ERASEBKGND:
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
