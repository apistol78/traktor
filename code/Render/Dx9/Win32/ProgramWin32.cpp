#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/CubeTextureDx9.h"
#include "Render/Dx9/HlslProgram.h"
#include "Render/Dx9/ParameterCache.h"
#include "Render/Dx9/ProgramResourceDx9.h"
#include "Render/Dx9/ResourceManagerDx9.h"
#include "Render/Dx9/ShaderCache.h"
#include "Render/Dx9/SimpleTextureDx9.h"
#include "Render/Dx9/VolumeTextureDx9.h"
#include "Render/Dx9/Win32/ProgramWin32.h"
#include "Render/Dx9/Win32/RenderTargetWin32.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

IDirect3DBaseTexture9* getD3DTexture(ITexture* texture)
{
	if (!texture)
		return 0;

	if (is_a< SimpleTextureDx9 >(texture))
		return static_cast< SimpleTextureDx9* >(texture)->getD3DBaseTexture();
	if (is_a< CubeTextureDx9 >(texture))
		return static_cast< CubeTextureDx9* >(texture)->getD3DBaseTexture();
	if (is_a< VolumeTextureDx9 >(texture))
		return static_cast< VolumeTextureDx9* >(texture)->getD3DBaseTexture();
	if (is_a< RenderTargetWin32 >(texture))
		return static_cast< RenderTargetWin32* >(texture)->getD3DBaseTexture();

	return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramWin32", ProgramWin32, IProgram)

ProgramWin32* ProgramWin32::ms_activeProgram = 0;

ProgramWin32::ProgramWin32(ResourceManagerDx9* resourceManager, ParameterCache* parameterCache)
:	m_resourceManager(resourceManager)
,	m_parameterCache(parameterCache)
,	m_dirty(true)
{
	m_resourceManager->add(this);
}

ProgramWin32::~ProgramWin32()
{
	destroy();
}

bool ProgramWin32::create(
	IDirect3DDevice9* d3dDevice,
	ShaderCache* shaderCache,
	const ProgramResourceDx9* resource,
	int32_t maxAnisotropy
)
{
	T_ASSERT (d3dDevice);
	HRESULT hr;

	m_d3dVertexShader = shaderCache->getVertexShader(resource->m_vertexShaderHash);
	if (!m_d3dVertexShader)
	{
		hr = d3dDevice->CreateVertexShader(
			(const DWORD *)resource->m_vertexShader->GetBufferPointer(),
			&m_d3dVertexShader.getAssign()
		);
		if (FAILED(hr))
		{
			log::error << L"Unable to create program; CreateVertexShader failed " << int32_t(hr) << Endl;
			return false;
		}
		shaderCache->putVertexShader(resource->m_vertexShaderHash, m_d3dVertexShader);
	}

	m_d3dPixelShader = shaderCache->getPixelShader(resource->m_pixelShaderHash);
	if (!m_d3dPixelShader)
	{
		hr = d3dDevice->CreatePixelShader(
			(const DWORD *)resource->m_pixelShader->GetBufferPointer(),
			&m_d3dPixelShader.getAssign()
		);
		if (FAILED(hr))
		{
			log::error << L"Unable to create program; CreatePixelShader failed " << int32_t(hr) << Endl;
			return false;
		}
		shaderCache->putPixelShader(resource->m_pixelShaderHash, m_d3dPixelShader);
	}

	m_vertexScalars = resource->m_vertexScalars;
	m_pixelScalars = resource->m_pixelScalars;
	m_vertexSamplers = resource->m_vertexSamplers;
	m_pixelSamplers = resource->m_pixelSamplers;

	for (std::map< std::wstring, uint32_t >::const_iterator i = resource->m_scalarParameterMap.begin(); i != resource->m_scalarParameterMap.end(); ++i)
		m_scalarParameterMap.insert(std::make_pair(
			getParameterHandle(i->first),
			i->second
		));

	for (std::map< std::wstring, uint32_t >::const_iterator i = resource->m_textureParameterMap.begin(); i != resource->m_textureParameterMap.end(); ++i)
		m_textureParameterMap.insert(std::make_pair(
			getParameterHandle(i->first),
			i->second
		));

	m_scalarParameterData.resize(resource->m_scalarParameterDataSize);
	m_textureParameterData.resize(resource->m_textureParameterDataSize);

	m_state = resource->m_state;
	m_state.prepareAnisotropy(maxAnisotropy);

	m_d3dDevice = d3dDevice;

	return true;
}

bool ProgramWin32::activate()
{
	if (ms_activeProgram == this && !m_dirty)
		return true;

	m_parameterCache->setVertexShader(m_d3dVertexShader);
	m_parameterCache->setPixelShader(m_d3dPixelShader);
	m_state.apply(m_parameterCache);

	for (std::vector< ProgramScalar >::iterator i = m_vertexScalars.begin(); i != m_vertexScalars.end(); ++i)
		m_parameterCache->setVertexShaderConstant(i->registerIndex, i->registerCount, &m_scalarParameterData[i->offset]);

	for (std::vector< ProgramScalar >::iterator i = m_pixelScalars.begin(); i != m_pixelScalars.end(); ++i)
		m_parameterCache->setPixelShaderConstant(i->registerIndex, i->registerCount, &m_scalarParameterData[i->offset]);

	for (std::vector< ProgramSampler >::iterator i = m_vertexSamplers.begin(); i != m_vertexSamplers.end(); ++i)
		m_parameterCache->setVertexTexture(i->stage, getD3DTexture(m_textureParameterData[i->texture]));

	for (std::vector< ProgramSampler >::iterator i = m_pixelSamplers.begin(); i != m_pixelSamplers.end(); ++i)
		m_parameterCache->setPixelTexture(i->stage, getD3DTexture(m_textureParameterData[i->texture]));

	m_dirty = false;
	ms_activeProgram = this;
	return true;
}

void ProgramWin32::forceDirty()
{
	ms_activeProgram = 0;
}

void ProgramWin32::destroy()
{
	m_d3dDevice.release();
	m_d3dVertexShader.release();
	m_d3dPixelShader.release();
	m_resourceManager->remove(this);
}

void ProgramWin32::setFloatParameter(handle_t handle, float param)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	m_scalarParameterData[i->second] = param;
	m_dirty = true;
}

void ProgramWin32::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	std::memcpy(&m_scalarParameterData[i->second], param, length * sizeof(float));
	m_dirty = true;
}

void ProgramWin32::setVectorParameter(handle_t handle, const Vector4& param)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	param.storeUnaligned(&m_scalarParameterData[i->second]);
	m_dirty = true;
}

void ProgramWin32::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	for (int j = 0; j < length; ++j)
		param[j].storeUnaligned(&m_scalarParameterData[i->second + j * 4]);

	m_dirty = true;
}

void ProgramWin32::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	param.storeUnaligned(&m_scalarParameterData[i->second]);
	m_dirty = true;
}

void ProgramWin32::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	for (int j = 0; j < length; ++j)
		param[j].storeUnaligned(&m_scalarParameterData[i->second + j * 16]);

	m_dirty = true;
}

void ProgramWin32::setTextureParameter(handle_t handle, ITexture* texture)
{
	std::map< handle_t, uint32_t >::const_iterator i = m_textureParameterMap.find(handle);
	if (i == m_textureParameterMap.end() || m_textureParameterData[i->second] == texture)
		return;

	m_textureParameterData[i->second] = texture;
	m_dirty = true;
}

void ProgramWin32::setStencilReference(uint32_t stencilReference)
{
	m_state.setRenderState(D3DRS_STENCILREF, stencilReference);
	ms_activeProgram = 0;
}

HRESULT ProgramWin32::lostDevice()
{
	ms_activeProgram = 0;
	m_dirty = true;
	return S_OK;
}

HRESULT ProgramWin32::resetDevice(IDirect3DDevice9* d3dDevice)
{
	return S_OK;
}

	}
}
