#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/Dx9/CubeTextureDx9.h"
#include "Render/Dx9/ParameterCache.h"
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/ProgramResourceDx9.h"
#include "Render/Dx9/SimpleTextureDx9.h"
#include "Render/Dx9/VolumeTextureDx9.h"
#include "Render/Dx9/Xbox360/ProgramXbox360.h"
#include "Render/Dx9/Xbox360/RenderTargetXbox360.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

size_t collectParameters(
	ID3DXConstantTable* d3dConstantTable,
	std::map< handle_t, ProgramXbox360::Uniform >& outUniformMap,
	std::map< handle_t, ProgramXbox360::Sampler >& outSamplerMap
)
{
	D3DXCONSTANTTABLE_DESC dctd;
	D3DXCONSTANT_DESC dcd;
	size_t offset = 0;

	d3dConstantTable->GetDesc(&dctd);
	for (UINT i = 0; i < dctd.Constants; ++i)
	{
		D3DXHANDLE handle = d3dConstantTable->GetConstant(NULL, i);
		if (!handle)
			continue;
		
		UINT count = 1;
		if (FAILED(d3dConstantTable->GetConstantDesc(handle, &dcd, &count)))
			continue;

		if (dcd.Class == D3DXPC_SCALAR || dcd.Class == D3DXPC_VECTOR || dcd.Class == D3DXPC_MATRIX_COLUMNS)
		{
			T_ASSERT (dcd.Type == D3DXPT_FLOAT);

			handle_t handle = getParameterHandle(mbstows(dcd.Name));
			outUniformMap[handle].registerIndex = dcd.RegisterIndex;
			outUniformMap[handle].registerCount = dcd.RegisterCount;
			outUniformMap[handle].offset = 0;
			outUniformMap[handle].length = dcd.Rows * dcd.Columns * dcd.Elements;
		}
		else if (dcd.Class == D3DXPC_OBJECT)
		{
			if (dcd.Type != D3DXPT_SAMPLER2D && dcd.Type != D3DXPT_SAMPLER3D && dcd.Type != D3DXPT_SAMPLERCUBE)
				continue;

			handle_t handle = getParameterHandle(mbstows(dcd.Name));
			outSamplerMap[handle].stage = dcd.RegisterIndex;
			outSamplerMap[handle].texture = 0;
		}
	}

	return offset;
}

IDirect3DBaseTexture9* getD3DTexture(ITexture* texture)
{
	if (!texture)
		return 0;
/*
	if (is_a< SimpleTextureDx9 >(texture))
		return static_cast< TextureBaseDx9* >(static_cast< SimpleTextureDx9* >(texture))->getD3DBaseTexture();
	if (is_a< CubeTextureDx9 >(texture))
		return static_cast< TextureBaseDx9* >(static_cast< CubeTextureDx9* >(texture))->getD3DBaseTexture();
	if (is_a< VolumeTextureDx9 >(texture))
		return static_cast< TextureBaseDx9* >(static_cast< VolumeTextureDx9* >(texture))->getD3DBaseTexture();
	if (is_a< RenderTargetXbox360 >(texture))
		return static_cast< TextureBaseDx9* >(static_cast< RenderTargetXbox360* >(texture))->getD3DBaseTexture();
*/
	return 0;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ProgramXbox360", ProgramXbox360, IProgram)

ProgramXbox360* ProgramXbox360::ms_activeProgram = 0;

ProgramXbox360::ProgramXbox360(ParameterCache* parameterCache)
:	m_parameterCache(parameterCache)
,	m_dirty(true)
{
}

ProgramXbox360::~ProgramXbox360()
{
	destroy();
}

bool ProgramXbox360::create(
	IDirect3DDevice9* d3dDevice,
	const ProgramResourceDx9* resource
)
{
	ComRef< ID3DXConstantTable > d3dVertexConstantTable;
	ComRef< ID3DXConstantTable > d3dPixelConstantTable;
	HRESULT hr;

	T_ASSERT (d3dDevice);

	hr = d3dDevice->CreateVertexShader(
		(const DWORD *)resource->m_vertexShader->GetBufferPointer(),
		&m_d3dVertexShader.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create program; CreateVertexShader failed " << int32_t(hr) << Endl;
		return false;
	}
	
	hr = d3dDevice->CreatePixelShader(
		(const DWORD *)resource->m_pixelShader->GetBufferPointer(),
		&m_d3dPixelShader.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create program; CreatePixelShader failed " << int32_t(hr) << Endl;
		return false;
	}

	D3DXGetShaderConstantTable((const DWORD *)resource->m_vertexShader->GetBufferPointer(), &d3dVertexConstantTable.getAssign());
	D3DXGetShaderConstantTable((const DWORD *)resource->m_pixelShader->GetBufferPointer(), &d3dPixelConstantTable.getAssign());

	std::map< handle_t, Uniform > vertexUniformMap, pixelUniformMap;
	std::map< handle_t, Sampler > vertexSamplerMap, pixelSamplerMap;

	collectParameters(d3dVertexConstantTable, vertexUniformMap, vertexSamplerMap);
	collectParameters(d3dPixelConstantTable, pixelUniformMap, pixelSamplerMap);

	//// Build parameter map and allocate uniform data.
	//for (std::map< handle_t, Uniform >::iterator j = vertexUniformMap.begin(); j != vertexUniformMap.end(); ++j)
	//{
	//	uint32_t index = m_parameterMap.get(j->first);
	//	if (index != ~0U)
	//		j->second.offset = index;
	//	else
	//	{
	//		uint16_t offset = uint32_t(m_uniformFloatData.size());
	//		j->second.offset = offset;
	//		m_parameterMap.set(j->first, offset);
	//		m_uniformFloatData.resize(offset + j->second.length);
	//	}
	//	m_vertexUniforms.push_back(j->second);
	//}

	//// Build parameter map and allocate uniform data.
	//for (std::map< handle_t, Uniform >::iterator j = pixelUniformMap.begin(); j != pixelUniformMap.end(); ++j)
	//{
	//	uint32_t index = m_parameterMap.get(j->first);
	//	if (index != ~0U)
	//		j->second.offset = index;
	//	else
	//	{
	//		uint16_t offset = uint32_t(m_uniformFloatData.size());
	//		j->second.offset = offset;
	//		m_parameterMap.set(j->first, offset);
	//		m_uniformFloatData.resize(offset + j->second.length);
	//	}
	//	m_pixelUniforms.push_back(j->second);
	//}

	//// Build parameter map and texture array.
	//for (std::map< handle_t, Sampler >::iterator j = vertexSamplerMap.begin(); j != vertexSamplerMap.end(); ++j)
	//{
	//	uint32_t index = m_parameterMap.get(j->first);
	//	if (index != ~0U)
	//		j->second.texture = index;
	//	else
	//	{
	//		uint16_t texture = uint16_t(m_samplerTextures.size());
	//		j->second.texture = texture;
	//		m_parameterMap.set(j->first, texture);
	//		m_samplerTextures.resize(texture + 1);
	//	}
	//	m_vertexSamplers.push_back(j->second);
	//}

	//// Build parameter map and texture array.
	//for (std::map< handle_t, Sampler >::iterator j = pixelSamplerMap.begin(); j != pixelSamplerMap.end(); ++j)
	//{
	//	uint32_t index = m_parameterMap.get(j->first);
	//	if (index != ~0U)
	//		j->second.texture = index;
	//	else
	//	{
	//		uint16_t texture = uint16_t(m_samplerTextures.size());
	//		j->second.texture = texture;
	//		m_parameterMap.set(j->first, texture);
	//		m_samplerTextures.resize(texture + 1);
	//	}
	//	m_pixelSamplers.push_back(j->second);
	//}

	m_state = resource->m_state;
	m_d3dDevice = d3dDevice;

	return true;
}

bool ProgramXbox360::activate()
{
	if (ms_activeProgram == this && !m_dirty)
		return true;

	if (ms_activeProgram != this)
	{
		m_d3dDevice->SetVertexShader(m_d3dVertexShader);
		m_d3dDevice->SetPixelShader(m_d3dPixelShader);
		m_state.apply(m_parameterCache);
	}

	for (std::vector< Uniform >::iterator i = m_vertexUniforms.begin(); i != m_vertexUniforms.end(); ++i)
		m_parameterCache->setVertexShaderConstant(i->registerIndex, i->registerCount, &m_uniformFloatData[i->offset]);

	for (std::vector< Uniform >::iterator i = m_pixelUniforms.begin(); i != m_pixelUniforms.end(); ++i)
		m_parameterCache->setPixelShaderConstant(i->registerIndex, i->registerCount, &m_uniformFloatData[i->offset]);

	for (std::vector< Sampler >::iterator i = m_vertexSamplers.begin(); i != m_vertexSamplers.end(); ++i)
		m_parameterCache->setVertexTexture(i->stage, getD3DTexture(m_samplerTextures[i->texture]));

	for (std::vector< Sampler >::iterator i = m_pixelSamplers.begin(); i != m_pixelSamplers.end(); ++i)
		m_parameterCache->setPixelTexture(i->stage, getD3DTexture(m_samplerTextures[i->texture]));

	m_dirty = false;
	ms_activeProgram = this;
	return true;
}

void ProgramXbox360::destroy()
{
	if (ms_activeProgram == this)
	{
		m_d3dDevice->SetVertexShader(0);
		m_d3dDevice->SetPixelShader(0);
		ms_activeProgram = 0;
	}

#if !defined(_XBOX) && !defined(T_USE_XDK)
	m_context->releaseComRef(m_d3dDevice);
	m_context->releaseComRef(m_d3dVertexShader);
	m_context->releaseComRef(m_d3dPixelShader);
#endif
}

void ProgramXbox360::setFloatParameter(handle_t handle, float param)
{
	/*
	uint32_t index = m_parameterMap.get(handle);
	if (index == ~0U || m_uniformFloatData[index] == param)
		return;

	m_uniformFloatData[index] = param;
	m_dirty = true;
	*/
}

void ProgramXbox360::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	/*
	uint32_t index = m_parameterMap.get(handle);
	if (index == ~0U)
		return;

	std::memcpy(&m_uniformFloatData[index], param, length * sizeof(float));
	m_dirty = true;
	*/
}

void ProgramXbox360::setVectorParameter(handle_t handle, const Vector4& param)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(&param), 4);
}

void ProgramXbox360::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(param), length * 4);
}

void ProgramXbox360::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(&param), 16);
}

void ProgramXbox360::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	setFloatArrayParameter(handle, reinterpret_cast< const float* >(param), length * 16);
}

void ProgramXbox360::setTextureParameter(handle_t handle, ITexture* texture)
{
	/*
	uint32_t index = m_parameterMap.get(handle);
	if (index == ~0U || m_samplerTextures[index] == texture)
		return;

	m_samplerTextures[index] = texture;
	m_dirty = true;
	*/
}

void ProgramXbox360::setStencilReference(uint32_t stencilReference)
{
	m_state.setRenderState(D3DRS_STENCILREF, stencilReference);
	ms_activeProgram = 0;
}

bool ProgramXbox360::isOpaque() const
{
	return m_state.isOpaque();
}

void ProgramXbox360::forceDirty()
{
	ms_activeProgram = 0;
}

HRESULT ProgramXbox360::lostDevice()
{
	ms_activeProgram = 0;
	m_dirty = true;
	return S_OK;
}

HRESULT ProgramXbox360::resetDevice(IDirect3DDevice9* d3dDevice)
{
	return S_OK;
}

	}
}
