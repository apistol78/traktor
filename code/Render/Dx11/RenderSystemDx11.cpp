#include "Core/Log/Log.h"
#include "Render/VertexElement.h"
#include "Render/Dx11/RenderSystemDx11.h"
#include "Render/Dx11/RenderViewDx11.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/VertexBufferDx11.h"
#include "Render/Dx11/IndexBufferDx11.h"
#include "Render/Dx11/SimpleTextureDx11.h"
#include "Render/Dx11/CubeTextureDx11.h"
#include "Render/Dx11/VolumeTextureDx11.h"
#include "Render/Dx11/RenderTargetSetDx11.h"
#include "Render/Dx11/ProgramCompilerDx11.h"
#include "Render/Dx11/ProgramResourceDx11.h"
#include "Render/Dx11/ProgramDx11.h"
#include "Render/Dx11/TypesDx11.h"
#include "Render/Dx11/Utilities.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const TCHAR* c_className = _T("TraktorRenderSystem");

void setWindowStyle(HWND hWnd, int32_t clientWidth, int32_t clientHeight, bool fullScreen)
{
	if (fullScreen)
	{
		SetWindowLong(hWnd, GWL_STYLE, WS_POPUPWINDOW);
		SetWindowPos(hWnd, HWND_TOP, 0, 0, clientWidth, clientHeight, SWP_FRAMECHANGED | SWP_NOMOVE);
	}
	else
	{
		SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX);
		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, clientWidth, clientHeight, SWP_FRAMECHANGED | SWP_NOMOVE);
	}

	RECT rcWindow, rcClient;
	GetWindowRect(hWnd, &rcWindow);
	GetClientRect(hWnd, &rcClient);

	int32_t windowWidth = rcWindow.right - rcWindow.left;
	int32_t windowHeight = rcWindow.bottom - rcWindow.top;

	int32_t realClientWidth = rcClient.right - rcClient.left;
	int32_t realClientHeight = rcClient.bottom - rcClient.top;

	windowWidth = (windowWidth - realClientWidth) + clientWidth;
	windowHeight = (windowHeight - realClientHeight) + clientHeight;

	SetWindowPos(hWnd, NULL, 0, 0, windowWidth, windowHeight, SWP_NOZORDER | SWP_NOMOVE);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemDx11", 0, RenderSystemDx11, IRenderSystem)

RenderSystemDx11::RenderSystemDx11()
:	m_mipBias(0.0f)
{
}

bool RenderSystemDx11::create(const RenderSystemCreateDesc& desc)
{
	ComRef< IDXGIDevice > dxgiDevice;
	ComRef< IDXGIAdapter > dxgiAdapter;
	ComRef< IDXGIOutput > dxgiOutput;
	D3D_FEATURE_LEVEL d3dFeatureLevel;
	HRESULT hr;

	hr = D3D11CreateDevice(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
#if defined(_DEBUG)
		D3D11_CREATE_DEVICE_DEBUG,
#else
		0,
#endif
		0,
		0,
		D3D11_SDK_VERSION,
		&m_d3dDevice.getAssign(),
		&d3dFeatureLevel,
		&m_d3dDeviceContext.getAssign()
	);
	if (FAILED(hr))
		return false;

	hr = CreateDXGIFactory(
		__uuidof(IDXGIFactory),
		(void**)(&m_dxgiFactory.getAssign())
	);
	if (FAILED(hr))
		return false;

	hr = m_d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice.getAssign());
	if (FAILED(hr))
		return 0;

	hr = dxgiDevice->GetAdapter(&dxgiAdapter.getAssign());
	if (FAILED(hr))
		return 0;

	hr = dxgiAdapter->EnumOutputs(0, &dxgiOutput.getAssign());
	if (FAILED(hr))
		return 0;

	UINT count = 0;
	hr = dxgiOutput->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0,
		&count,
		0
	);
	if (FAILED(hr))
		return 0;

	m_dxgiDisplayModes.reset(new DXGI_MODE_DESC [count]); 
	hr = dxgiOutput->GetDisplayModeList(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		0,
		&count,
		m_dxgiDisplayModes.ptr()
	);
	if (FAILED(hr))
		return 0;

	m_displayModes.resize(count);
	for (UINT i = 0; i < count; ++i)
	{
		m_displayModes[i].width = m_dxgiDisplayModes[i].Width;
		m_displayModes[i].height = m_dxgiDisplayModes[i].Height;
		m_displayModes[i].refreshRate = m_dxgiDisplayModes[i].RefreshRate.Numerator;
		m_displayModes[i].colorBits = 32;
	}

	// Render window class.
	WNDCLASS wc;
	std::memset(&wc, 0, sizeof(wc));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(this);
	wc.lpfnWndProc = (WNDPROC)wndProc;
	wc.hInstance = static_cast<HINSTANCE>(GetModuleHandle(0));
	wc.hIcon = NULL;
	wc.hCursor = static_cast<HCURSOR>(LoadCursor(NULL, IDC_ARROW));
	wc.lpszClassName = c_className;
	RegisterClass(&wc);

	// Render window.
	m_hWnd = CreateWindow(
		c_className,
		desc.windowTitle ? desc.windowTitle : _T("Traktor 2.0 DirectX 10.0 Renderer"),
		WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		64,
		64,
		NULL,
		NULL,
		static_cast< HMODULE >(GetModuleHandle(NULL)),
		this
	);
	if (!m_hWnd)
		return false;

	m_context = new ContextDx11(m_d3dDevice, m_d3dDeviceContext);
	m_mipBias = desc.mipBias;

	return true;
}

void RenderSystemDx11::destroy()
{
	if (m_context)
	{
		m_context->deleteResources();
		m_context = 0;
	}

	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}

	m_dxgiFactory.release();
	m_d3dDevice.release();
}

uint32_t RenderSystemDx11::getDisplayModeCount() const
{
	return uint32_t(m_displayModes.size());
}

DisplayMode RenderSystemDx11::getDisplayMode(uint32_t index) const
{
	return m_displayModes[index];
}

DisplayMode RenderSystemDx11::getCurrentDisplayMode() const
{
	return DisplayMode();
}

float RenderSystemDx11::getDisplayAspectRatio() const
{
	return 0.0f;
}

bool RenderSystemDx11::handleMessages()
{
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
}

Ref< IRenderView > RenderSystemDx11::createRenderView(const RenderViewDefaultDesc& desc)
{
	ComRef< IDXGISwapChain > d3dSwapChain;
	DXGI_SWAP_CHAIN_DESC scd;
	DXGI_MODE_DESC* dmd;
	HRESULT hr;

	setWindowStyle(m_hWnd, desc.displayMode.width, desc.displayMode.height, desc.fullscreen);
	ShowWindow(m_hWnd, SW_NORMAL);

	// Find matching display mode.
	dmd = 0;
	for (uint32_t i = 0; i < m_displayModes.size(); ++i)
	{
		const DisplayMode& dm = m_displayModes[i];
		if (dm.width == desc.displayMode.width && dm.height == desc.displayMode.height)
		{
			if (desc.displayMode.refreshRate != 0 && dm.refreshRate != desc.displayMode.refreshRate)
				continue;

			dmd = &m_dxgiDisplayModes[i];
			break;
		}
	}
	if (!dmd)
	{
		log::error << L"Unable to create render view; display mode not supported" << Endl;
		return 0;
	}

	std::memset(&scd, 0, sizeof(scd));
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.BufferCount = 1;
	std::memcpy(&scd.BufferDesc, dmd, sizeof(DXGI_MODE_DESC));
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = m_hWnd;
	scd.Windowed = desc.fullscreen ? FALSE : TRUE;

	if (!setupSampleDesc(m_d3dDevice, desc.multiSample, scd.BufferDesc.Format, DXGI_FORMAT_D16_UNORM, scd.SampleDesc))
	{
		log::error << L"Unable to create render view; unsupported MSAA" << Endl;
		return 0;
	}

	hr = m_dxgiFactory->CreateSwapChain(
		m_d3dDevice,
		&scd,
		&d3dSwapChain.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; CreateSwapChain failed" << Endl;
		return 0;
	}

	return new RenderViewDx11(
		m_context,
		d3dSwapChain,
		scd,
		desc.waitVBlank
	);
}

Ref< IRenderView > RenderSystemDx11::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	ComRef< IDXGISwapChain > d3dSwapChain;
	HRESULT hr;

	RECT rc;
	GetClientRect((HWND)desc.nativeWindowHandle, &rc);

	if (rc.right <= rc.left)
		rc.right = 8;
	if (rc.bottom <= rc.top)
		rc.bottom = 8;

	DXGI_SWAP_CHAIN_DESC scd;
	std::memset(&scd, 0, sizeof(scd));
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.BufferCount = 1;
	scd.BufferDesc.Width = rc.right - rc.left;
	scd.BufferDesc.Height = rc.bottom - rc.top;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = (HWND)desc.nativeWindowHandle;
	scd.Windowed = TRUE;

	if (!setupSampleDesc(m_d3dDevice, desc.multiSample, scd.BufferDesc.Format, DXGI_FORMAT_D16_UNORM, scd.SampleDesc))
	{
		log::error << L"Unable to create render view; unsupported MSAA" << Endl;
		return 0;
	}

	hr = m_dxgiFactory->CreateSwapChain(
		m_d3dDevice,
		&scd,
		&d3dSwapChain.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; CreateSwapChain failed" << Endl;
		return 0;
	}

	return new RenderViewDx11(
		m_context,
		d3dSwapChain,
		scd,
		desc.waitVBlank
	);
}

Ref< VertexBuffer > RenderSystemDx11::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	ComRef< ID3D11Buffer > d3dBuffer;
	D3D11_BUFFER_DESC dbd;
	HRESULT hr;

	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D11_USAGE_DYNAMIC;
	dbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dbd.MiscFlags = 0;

	hr = m_d3dDevice->CreateBuffer(&dbd, NULL, &d3dBuffer.getAssign());
	if (FAILED(hr))
		return 0;

	std::vector< D3D11_INPUT_ELEMENT_DESC > d3dInputElements(vertexElements.size());
	for (unsigned int i = 0; i < vertexElements.size(); ++i)
	{
		T_ASSERT (vertexElements[i].getDataUsage() < sizeof_array(c_dxgiInputSemantic));
		T_ASSERT (vertexElements[i].getDataType() < sizeof_array(c_dxgiInputType));

		d3dInputElements[i].SemanticName = c_dxgiInputSemantic[vertexElements[i].getDataUsage()];
		d3dInputElements[i].SemanticIndex = vertexElements[i].getIndex();
		d3dInputElements[i].Format = c_dxgiInputType[vertexElements[i].getDataType()];
		d3dInputElements[i].InputSlot = 0;
		d3dInputElements[i].AlignedByteOffset = vertexElements[i].getOffset();
		d3dInputElements[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		d3dInputElements[i].InstanceDataStepRate = 0;
	}

	return new VertexBufferDx11(
		m_context,
		bufferSize,
		d3dBuffer,
		getVertexSize(vertexElements),
		d3dInputElements
	);
}

Ref< IndexBuffer > RenderSystemDx11::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	ComRef< ID3D11Buffer > d3dBuffer;
	D3D11_BUFFER_DESC dbd;
	HRESULT hr;

	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D11_USAGE_DYNAMIC;
	dbd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dbd.MiscFlags = 0;

	hr = m_d3dDevice->CreateBuffer(&dbd, NULL, &d3dBuffer.getAssign());
	if (FAILED(hr))
		return 0;

	return new IndexBufferDx11(m_context, indexType, bufferSize, d3dBuffer);
}

Ref< ISimpleTexture > RenderSystemDx11::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureDx11 > texture = new SimpleTextureDx11(m_context);
	if (!texture->create(desc))
		return 0;
	return texture;
}

Ref< ICubeTexture > RenderSystemDx11::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTextureDx11 > texture = new CubeTextureDx11(m_context);
	if (!texture->create(desc))
		return 0;
	return texture;
}

Ref< IVolumeTexture > RenderSystemDx11::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	return new VolumeTextureDx11();
}

Ref< RenderTargetSet > RenderSystemDx11::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetDx11 > renderTargetSet = new RenderTargetSetDx11(m_context);
	if (!renderTargetSet->create(m_d3dDevice, desc))
		return 0;
	return renderTargetSet;
}

Ref< IProgram > RenderSystemDx11::createProgram(const ProgramResource* programResource)
{
	Ref< const ProgramResourceDx11 > resource = dynamic_type_cast< const ProgramResourceDx11* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramDx11 > program = new ProgramDx11(m_context);
	if (!program->create(m_d3dDevice, resource, m_mipBias))
		return 0;

	return program;
}

Ref< IProgramCompiler > RenderSystemDx11::createProgramCompiler() const
{
	return new ProgramCompilerDx11();
}

void RenderSystemDx11::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

LRESULT RenderSystemDx11::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RenderSystemDx11* renderSystem = reinterpret_cast< RenderSystemDx11* >(GetWindowLongPtr(hWnd, 0));
	LPCREATESTRUCT createStruct;
	LRESULT result = TRUE;

	switch (uMsg)
	{
	case WM_CREATE:
		createStruct = reinterpret_cast< LPCREATESTRUCT >(lParam);
		renderSystem = reinterpret_cast< RenderSystemDx11* >(createStruct->lpCreateParams);
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

	}
}
