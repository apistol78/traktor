#include "Render/Dx10/RenderSystemDx10.h"
#include "Render/Dx10/RenderViewDx10.h"
#include "Render/Dx10/ContextDx10.h"
#include "Render/Dx10/VertexBufferDx10.h"
#include "Render/Dx10/IndexBufferDx10.h"
#include "Render/Dx10/SimpleTextureDx10.h"
#include "Render/Dx10/CubeTextureDx10.h"
#include "Render/Dx10/VolumeTextureDx10.h"
#include "Render/Dx10/RenderTargetSetDx10.h"
#include "Render/Dx10/ProgramResourceDx10.h"
#include "Render/Dx10/ProgramDx10.h"
#include "Render/Dx10/Hlsl.h"
#include "Render/Dx10/HlslProgram.h"
#include "Render/Dx10/TypesDx10.h"
#include "Render/Dx10/Utilities.h"
#include "Render/DisplayMode.h"
#include "Render/VertexElement.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const TCHAR* c_className = _T("TraktorRenderSystem");

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.RenderSystemDx10", RenderSystemDx10, IRenderSystem)

bool RenderSystemDx10::create()
{
	ComRef< IDXGIDevice > dxgiDevice;
	ComRef< IDXGIAdapter > dxgiAdapter;
	ComRef< IDXGIOutput > dxgiOutput;
	HRESULT hr;

	hr = D3D10CreateDevice(
		NULL,
		D3D10_DRIVER_TYPE_HARDWARE,
		NULL,
#if defined(_DEBUG)
		D3D10_CREATE_DEVICE_DEBUG,
#else
		0,
#endif
		D3D10_SDK_VERSION,
		&m_d3dDevice.getAssign()
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

	for (UINT j = 0; j < count; ++j)
	{
		m_displayModes.push_back(gc_new< DisplayMode >(
			j,
			m_dxgiDisplayModes[j].Width,
			m_dxgiDisplayModes[j].Height,
			32
		));
	}

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

	m_hWnd = CreateWindow(
		c_className,
		_T("Traktor 2.0 DirectX 10.0 Renderer"),
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
		return false;

	m_context = gc_new< ContextDx10 >();
	return true;
}

void RenderSystemDx10::destroy()
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

int RenderSystemDx10::getDisplayModeCount() const
{
	return int(m_displayModes.size());
}

Ref< DisplayMode > RenderSystemDx10::getDisplayMode(int index)
{
	T_ASSERT (index >= 0 && index < int(m_displayModes.size()));
	return m_displayModes[index];
}

Ref< DisplayMode > RenderSystemDx10::getCurrentDisplayMode()
{
	return 0;
}

bool RenderSystemDx10::handleMessages()
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

Ref< IRenderView > RenderSystemDx10::createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc)
{
	ComRef< IDXGISwapChain > d3dSwapChain;
	DXGI_SWAP_CHAIN_DESC scd;
	DXGI_MODE_DESC* dmd;
	HRESULT hr;

	SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, displayMode->getWidth(), displayMode->getHeight(), SWP_SHOWWINDOW);
	ShowWindow(m_hWnd, SW_MAXIMIZE);
	UpdateWindow(m_hWnd);

	dmd = &m_dxgiDisplayModes[displayMode->getIndex()];
	T_ASSERT (dmd);

	std::memset(&scd, 0, sizeof(scd));
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.BufferCount = 1;
	std::memcpy(&scd.BufferDesc, dmd, sizeof(DXGI_MODE_DESC));
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = m_hWnd;
	scd.Windowed = FALSE;
	
	if (desc.multiSample > 0)
	{
		scd.SampleDesc.Count = 0;
		for (uint32_t i = 1; i <= D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT; ++i)
		{
			UINT msQuality1 = 0;
			UINT msQuality2 = 0;

			hr = m_d3dDevice->CheckMultisampleQualityLevels(
				scd.BufferDesc.Format,
				i,
				&msQuality1
			);

			if (SUCCEEDED(hr) && msQuality1 > 0)
			{
				hr = m_d3dDevice->CheckMultisampleQualityLevels(
					DXGI_FORMAT_D16_UNORM,
					i,
					&msQuality2
				);

				if (SUCCEEDED(hr) && msQuality2 > 0)
				{
					scd.SampleDesc.Count = i;
					scd.SampleDesc.Quality = min(msQuality1 - 1, msQuality2 - 1);
				}
			}
		}
		if (!scd.SampleDesc.Count)
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

	return gc_new< RenderViewDx10 >(
		m_context,
		m_d3dDevice,
		d3dSwapChain,
		cref(scd),
		desc.waitVBlank
	);
}

Ref< IRenderView > RenderSystemDx10::createRenderView(void* windowHandle, const RenderViewCreateDesc& desc)
{
	ComRef< IDXGISwapChain > d3dSwapChain;
	HRESULT hr;

	RECT rc;
	GetClientRect((HWND)windowHandle, &rc);

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
	scd.OutputWindow = (HWND)windowHandle;
	scd.Windowed = TRUE;

	if (!setupSampleDesc(m_d3dDevice, desc.multiSample, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D16_UNORM, scd.SampleDesc))
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

	return gc_new< RenderViewDx10 >(
		m_context,
		m_d3dDevice,
		d3dSwapChain,
		cref(scd),
		desc.waitVBlank
	);
}

Ref< VertexBuffer > RenderSystemDx10::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	ComRef< ID3D10Buffer > d3dBuffer;
	D3D10_BUFFER_DESC dbd;
	HRESULT hr;

	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D10_USAGE_DYNAMIC;
	dbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	dbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	dbd.MiscFlags = 0;

	hr = m_d3dDevice->CreateBuffer(&dbd, NULL, &d3dBuffer.getAssign());
	if (FAILED(hr))
		return 0;

	std::vector< D3D10_INPUT_ELEMENT_DESC > d3dInputElements(vertexElements.size());
	for (unsigned int i = 0; i < vertexElements.size(); ++i)
	{
		T_ASSERT (vertexElements[i].getDataUsage() < sizeof_array(c_dxgiInputSemantic));
		T_ASSERT (vertexElements[i].getDataType() < sizeof_array(c_dxgiInputType));

		d3dInputElements[i].SemanticName = c_dxgiInputSemantic[vertexElements[i].getDataUsage()];
		d3dInputElements[i].SemanticIndex = vertexElements[i].getIndex();
		d3dInputElements[i].Format = c_dxgiInputType[vertexElements[i].getDataType()];
		d3dInputElements[i].InputSlot = 0;
		d3dInputElements[i].AlignedByteOffset = vertexElements[i].getOffset();
		d3dInputElements[i].InputSlotClass = D3D10_INPUT_PER_VERTEX_DATA;
		d3dInputElements[i].InstanceDataStepRate = 0;
	}

	return gc_new< VertexBufferDx10 >(
		m_context,
		bufferSize,
		d3dBuffer,
		getVertexSize(vertexElements),
		cref(d3dInputElements)
	);
}

Ref< IndexBuffer > RenderSystemDx10::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	ComRef< ID3D10Buffer > d3dBuffer;
	D3D10_BUFFER_DESC dbd;
	HRESULT hr;

	dbd.ByteWidth = bufferSize;
	dbd.Usage = D3D10_USAGE_DYNAMIC;
	dbd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	dbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	dbd.MiscFlags = 0;

	hr = m_d3dDevice->CreateBuffer(&dbd, NULL, &d3dBuffer.getAssign());
	if (FAILED(hr))
		return 0;

	return gc_new< IndexBufferDx10 >(m_context, indexType, bufferSize, d3dBuffer);
}

Ref< ISimpleTexture > RenderSystemDx10::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureDx10 > texture = gc_new< SimpleTextureDx10 >(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

Ref< ICubeTexture > RenderSystemDx10::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTextureDx10 > texture = gc_new< CubeTextureDx10 >(m_context);
	if (!texture->create(m_d3dDevice, desc))
		return 0;
	return texture;
}

Ref< IVolumeTexture > RenderSystemDx10::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	return gc_new< VolumeTextureDx10 >();
}

Ref< RenderTargetSet > RenderSystemDx10::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetDx10 > renderTargetSet = gc_new< RenderTargetSetDx10 >(m_context);
	if (!renderTargetSet->create(m_d3dDevice, desc))
		return 0;
	return renderTargetSet;
}

Ref< ProgramResource > RenderSystemDx10::compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate)
{
	HlslProgram hlslProgram;
	if (!Hlsl().generate(shaderGraph, hlslProgram))
		return 0;

	Ref< ProgramResourceDx10 > programResource = ProgramDx10::compile(m_d3dDevice, hlslProgram);
	if (!programResource)
		return 0;

	return programResource;
}

Ref< IProgram > RenderSystemDx10::createProgram(const ProgramResource* programResource)
{
	Ref< const ProgramResourceDx10 > resource = dynamic_type_cast< const ProgramResourceDx10* >(programResource);
	if (!resource)
		return 0;

	Ref< ProgramDx10 > program = gc_new< ProgramDx10 >(m_context);
	if (!program->create(m_d3dDevice, resource))
		return 0;

	return program;
}

LRESULT RenderSystemDx10::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RenderSystemDx10* renderSystem = reinterpret_cast< RenderSystemDx10* >(GetWindowLongPtr(hWnd, 0));
	LPCREATESTRUCT createStruct;
	LRESULT result = TRUE;

	switch (uMsg)
	{
	case WM_CREATE:
		createStruct = reinterpret_cast< LPCREATESTRUCT >(lParam);
		renderSystem = reinterpret_cast< RenderSystemDx10* >(createStruct->lpCreateParams);
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
