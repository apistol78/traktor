#include "Render/Sw/RenderSystemSw.h"
#include "Render/Sw/RenderViewSw.h"
#include "Render/Sw/VertexBufferSw.h"
#include "Render/Sw/IndexBufferSw.h"
#include "Render/Sw/SimpleTextureSw.h"
#include "Render/Sw/RenderTargetSetSw.h"
#include "Render/Sw/ProgramResourceSw.h"
#include "Render/Sw/ProgramSw.h"
#include "Render/Sw/Emitter/Emitter.h"
#include "Render/Sw/Emitter/EmitterContext.h"
#include "Render/DisplayMode.h"
#include "Render/ShaderGraph.h"
#include "Render/VertexElement.h"
#include "Graphics/GraphicsSystem.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

#if defined(_WIN32)
#	if !defined(WINCE)
#		include "Graphics/Dd7/GraphicsSystemDd7.h"
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

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.RenderSystemSw", RenderSystemSw, IRenderSystem)

RenderSystemSw::RenderSystemSw()
#if defined(_WIN32)
:	m_hWnd(NULL)
#endif
{
	m_processor = gc_new< ProcessorImpl >();
}

bool RenderSystemSw::create()
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

	Ref< graphics::GraphicsSystem > graphicsSystem;

#if defined(_WIN32)
#	if !defined(WINCE)
	graphicsSystem = gc_new< graphics::GraphicsSystemDd7 >();
#	else
	graphicsSystem = gc_new< graphics::GraphicsSystemDdWm5 >();
#	endif
#endif

	if (graphicsSystem)
	{
		graphicsSystem->getDisplayModes(m_displayModes);
		graphicsSystem->destroy();
	}

#endif
	return true;
}

void RenderSystemSw::destroy()
{
}

int RenderSystemSw::getDisplayModeCount() const
{
	return int(m_displayModes.size());
}

Ref< DisplayMode > RenderSystemSw::getDisplayMode(int index)
{
	return gc_new< DisplayMode >(
		index,
		m_displayModes[index].width,
		m_displayModes[index].height,
		m_displayModes[index].bits
	);
}

Ref< DisplayMode > RenderSystemSw::getCurrentDisplayMode()
{
	return 0;
}

bool RenderSystemSw::handleMessages()
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

#else
	return false;
#endif
}

Ref< IRenderView > RenderSystemSw::createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc)
{
	Ref< graphics::GraphicsSystem > graphicsSystem;
	graphics::CreateDesc graphicsDesc;

#if defined(_WIN32)

	m_hWnd = CreateWindow(
		c_windowClassName,
		_T("Traktor 2.0 Software Renderer"),
		WS_POPUP,
		0,
		0,
		displayMode->getWidth(),
		displayMode->getHeight(),
		NULL,
		NULL,
		static_cast< HMODULE >(GetModuleHandle(NULL)),
		this
	);
	if (!m_hWnd)
		return 0;

	SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	ShowWindow(m_hWnd, SW_MAXIMIZE);
	UpdateWindow(m_hWnd);

#endif

#if defined(_WIN32)
#	if !defined(WINCE)
	graphicsSystem = gc_new< graphics::GraphicsSystemDd7 >();
#	else
	graphicsSystem = gc_new< graphics::GraphicsSystemDdWm5 >();
#	endif
#endif

	graphicsDesc.windowHandle = m_hWnd;
	graphicsDesc.fullScreen = true;
	graphicsDesc.displayMode.width = displayMode->getWidth();
	graphicsDesc.displayMode.height = displayMode->getHeight();
	graphicsDesc.displayMode.bits = displayMode->getColorBits();
	graphicsDesc.pixelFormat = graphics::PfeR5G6B5;

	if (!graphicsSystem->create(graphicsDesc))
		return 0;

	return gc_new< RenderViewSw >(this, graphicsSystem, m_processor);
}

Ref< IRenderView > RenderSystemSw::createRenderView(void* windowHandle, const RenderViewCreateDesc& desc)
{
	Ref< graphics::GraphicsSystem > graphicsSystem;
	graphics::CreateDesc graphicsDesc;

	graphicsDesc.windowHandle = windowHandle;
	graphicsDesc.fullScreen = false;
	graphicsDesc.displayMode.width = 16;
	graphicsDesc.displayMode.height = 16;
	graphicsDesc.displayMode.bits = 16;
	graphicsDesc.pixelFormat = graphics::PfeR5G6B5;

#if defined(_WIN32)
#	if !defined(WINCE)
	graphicsSystem = gc_new< graphics::GraphicsSystemDd7 >();
#	else
	graphicsSystem = gc_new< graphics::GraphicsSystemDdWm5 >();
#	endif

	RECT rc;
	GetClientRect((HWND)windowHandle, &rc);
	graphicsDesc.displayMode.width = std::max(int(rc.right - rc.left), 16);
	graphicsDesc.displayMode.height = std::max(int(rc.bottom - rc.top), 16);

#endif

	if (!graphicsSystem->create(graphicsDesc))
		return 0;

	return gc_new< RenderViewSw >(this, graphicsSystem, m_processor);
}

Ref< VertexBuffer > RenderSystemSw::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	return gc_new< VertexBufferSw >(vertexElements, bufferSize);
}

Ref< IndexBuffer > RenderSystemSw::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	return gc_new< IndexBufferSw >(indexType, bufferSize);
}

Ref< ISimpleTexture > RenderSystemSw::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureSw > texture = gc_new< SimpleTextureSw >();
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
	Ref< RenderTargetSetSw > renderTargetSet = gc_new< RenderTargetSetSw >();
	if (!renderTargetSet->create(desc))
		return 0;
	return renderTargetSet;
}

Ref< ProgramResource > RenderSystemSw::compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate)
{
	// @fixme Currently just embedding shader graph in resource.
	return gc_new< ProgramResourceSw >(shaderGraph);
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
	std::map< handle_t, int > parameterMap;
	for (std::map< std::wstring, Variable* >::const_iterator i = parameters.uniforms.begin(); i != parameters.uniforms.end(); ++i)
		parameterMap[getParameterHandle(i->first)] = i->second->reg;

	std::map< handle_t, int > samplerMap;
	for (std::map< std::wstring, int >::const_iterator i = parameters.samplers.begin(); i != parameters.samplers.end(); ++i)
		samplerMap[getParameterHandle(i->first)] = i->second;

	return gc_new< ProgramSw >(
		cref(parameterMap),
		cref(samplerMap),
		vertexProgram,
		pixelProgram,
		cref(cx.getRenderState()),
		cx.getInterpolatorCount()
	);
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
