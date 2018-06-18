/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Math/Matrix44.h"
#include "Core/Misc/TString.h"
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/CubeTextureDx9.h"
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

void resolveTextureSize(ITexture* texture, float outTextureSize[4])
{
	if (!texture)
		return;

	Ref< ITexture > resolved = texture->resolve();
	if (!resolved)
		return;

	if (is_a< SimpleTextureDx9 >(resolved))
	{
		SimpleTextureDx9* simpleTexture = checked_type_cast< SimpleTextureDx9*, false >(resolved);
		outTextureSize[0] = float(simpleTexture->getWidth());
		outTextureSize[1] = float(simpleTexture->getHeight());
		outTextureSize[2] = 0.0f;
		outTextureSize[3] = 0.0f;
	}

	else if (is_a< CubeTextureDx9 >(resolved))
	{
		CubeTextureDx9* cubeTexture = checked_type_cast< CubeTextureDx9*, false >(resolved);
		outTextureSize[0] = float(cubeTexture->getWidth());
		outTextureSize[1] = float(cubeTexture->getHeight());
		outTextureSize[2] = float(cubeTexture->getDepth());
		outTextureSize[3] = 0.0f;
	}

	else if (is_a< VolumeTextureDx9 >(resolved))
	{
		VolumeTextureDx9* volumeTexture = checked_type_cast< VolumeTextureDx9*, false >(resolved);
		outTextureSize[0] = float(volumeTexture->getWidth());
		outTextureSize[1] = float(volumeTexture->getHeight());
		outTextureSize[2] = float(volumeTexture->getDepth());
		outTextureSize[3] = 0.0f;
	}

	else if (is_a< RenderTargetWin32 >(resolved))
	{
		RenderTargetWin32* renderTarget = checked_type_cast< RenderTargetWin32*, false >(resolved);
		outTextureSize[0] = float(renderTarget->getWidth());
		outTextureSize[1] = float(renderTarget->getHeight());
		outTextureSize[2] = 0.0f;
		outTextureSize[3] = 0.0f;
	}
}

IDirect3DBaseTexture9* resolveD3DTexture(ITexture* texture)
{
	if (!texture)
		return 0;

	Ref< ITexture > resolved = texture->resolve();
	if (!resolved)
		return 0;

	if (is_a< SimpleTextureDx9 >(resolved))
	{
		SimpleTextureDx9* simpleTexture = checked_type_cast< SimpleTextureDx9*, false >(resolved);
		return simpleTexture->getD3DBaseTexture();
	}

	else if (is_a< CubeTextureDx9 >(resolved))
	{
		CubeTextureDx9* cubeTexture = checked_type_cast< CubeTextureDx9*, false >(resolved);
		return cubeTexture->getD3DBaseTexture();
	}

	else if (is_a< VolumeTextureDx9 >(resolved))
	{
		VolumeTextureDx9* volumeTexture = checked_type_cast< VolumeTextureDx9*, false >(resolved);
		return volumeTexture->getD3DBaseTexture();
	}

	else if (is_a< RenderTargetWin32 >(resolved))
	{
		RenderTargetWin32* renderTarget = checked_type_cast< RenderTargetWin32*, false >(resolved);
		return renderTarget->getD3DBaseTexture();
	}

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

	m_resource = resource;

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

	m_scalarParameterMap.reserve(resource->m_scalarParameterMap.size());
	m_textureParameterMap.reserve(resource->m_textureParameterMap.size());

	for (std::map< std::wstring, uint32_t >::const_iterator i = resource->m_scalarParameterMap.begin(); i != resource->m_scalarParameterMap.end(); ++i)
	{
		handle_t handle = getParameterHandle(i->first);

		m_scalarParameterMap.insert(std::make_pair(
			handle,
			i->second
		));
#if defined(_DEBUG)
		m_scalarParameterNames.insert(std::make_pair(
			handle,
			i->first
		));
#endif
	}

	for (std::map< std::wstring, uint32_t >::const_iterator i = resource->m_textureParameterMap.begin(); i != resource->m_textureParameterMap.end(); ++i)
		m_textureParameterMap.insert(std::make_pair(
			getParameterHandle(i->first),
			i->second
		));

	m_scalarParameterData.resize(resource->m_scalarParameterDataSize);
	m_textureParameterData.resize(resource->m_textureParameterDataSize);

#if defined(_DEBUG)
	m_scalarParameterDataValid.resize(resource->m_scalarParameterDataSize, 0);
#endif

	m_state = resource->m_state;
	m_state.prepareAnisotropy(maxAnisotropy);

	m_d3dDevice = d3dDevice;

	return true;
}

bool ProgramWin32::activate()
{
	IDirect3DBaseTexture9* d3dTexture;
	float T_MATH_ALIGN16 textureSize[4];

	if (ms_activeProgram == this && !m_dirty)
		return true;

	m_parameterCache->setVertexShader(m_d3dVertexShader);
	m_parameterCache->setPixelShader(m_d3dPixelShader);
	m_state.apply(m_parameterCache);

	for (AlignedVector< ProgramScalar >::const_iterator i = m_resource->m_vertexScalars.begin(); i != m_resource->m_vertexScalars.end(); ++i)
	{
#if defined(_DEBUG)
		validateParameter(*i);
#endif
		m_parameterCache->setVertexShaderConstant(i->registerIndex, i->registerCount, &m_scalarParameterData[i->offset]);
	}

	for (AlignedVector< ProgramScalar >::const_iterator i = m_resource->m_pixelScalars.begin(); i != m_resource->m_pixelScalars.end(); ++i)
	{
#if defined(_DEBUG)
		validateParameter(*i);
#endif
		m_parameterCache->setPixelShaderConstant(i->registerIndex, i->registerCount, &m_scalarParameterData[i->offset]);
	}

	for (AlignedVector< ProgramTexture >::const_iterator i = m_resource->m_vertexTextures.begin(); i != m_resource->m_vertexTextures.end(); ++i)
	{
		resolveTextureSize(m_textureParameterData[i->texture], textureSize);
		m_parameterCache->setVertexShaderConstant(i->sizeIndex, 1, textureSize);
	}

	for (AlignedVector< ProgramTexture >::const_iterator i = m_resource->m_pixelTextures.begin(); i != m_resource->m_pixelTextures.end(); ++i)
	{
		resolveTextureSize(m_textureParameterData[i->texture], textureSize);
		m_parameterCache->setPixelShaderConstant(i->sizeIndex, 1, textureSize);
	}

	for (AlignedVector< ProgramSampler >::const_iterator i = m_resource->m_vertexSamplers.begin(); i != m_resource->m_vertexSamplers.end(); ++i)
	{
		d3dTexture = resolveD3DTexture(m_textureParameterData[i->texture]);
		m_parameterCache->setVertexTexture(i->stage, d3dTexture);
	}

	for (AlignedVector< ProgramSampler >::const_iterator i = m_resource->m_pixelSamplers.begin(); i != m_resource->m_pixelSamplers.end(); ++i)
	{
		d3dTexture = resolveD3DTexture(m_textureParameterData[i->texture]);
		m_parameterCache->setPixelTexture(i->stage, d3dTexture);
	}

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
	m_resource = 0;

	m_d3dDevice.release();
	m_d3dVertexShader.release();
	m_d3dPixelShader.release();

	m_resourceManager->remove(this);
}

void ProgramWin32::setFloatParameter(handle_t handle, float param)
{
	SmallMap< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	m_scalarParameterData[i->second] = param;
#if defined(_DEBUG)
	m_scalarParameterDataValid[i->second] = 1;
#endif
	m_dirty = true;
}

void ProgramWin32::setFloatArrayParameter(handle_t handle, const float* param, int length)
{
	SmallMap< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	std::memcpy(&m_scalarParameterData[i->second], param, length * sizeof(float));
#if defined(_DEBUG)
	for (int j = 0; j < length; ++j)
		m_scalarParameterDataValid[i->second + j] = 1;
#endif

	m_dirty = true;
}

void ProgramWin32::setVectorParameter(handle_t handle, const Vector4& param)
{
	SmallMap< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	param.storeAligned(&m_scalarParameterData[i->second]);
#if defined(_DEBUG)
	for (int j = 0; j < 4; ++j)
		m_scalarParameterDataValid[i->second + j] = 1;
#endif

	m_dirty = true;
}

void ProgramWin32::setVectorArrayParameter(handle_t handle, const Vector4* param, int length)
{
	SmallMap< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	for (int j = 0; j < length; ++j)
		param[j].storeAligned(&m_scalarParameterData[i->second + j * 4]);
#if defined(_DEBUG)
	for (int j = 0; j < length * 4; ++j)
		m_scalarParameterDataValid[i->second + j] = 1;
#endif

	m_dirty = true;
}

void ProgramWin32::setMatrixParameter(handle_t handle, const Matrix44& param)
{
	SmallMap< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	param.storeAligned(&m_scalarParameterData[i->second]);
#if defined(_DEBUG)
	for (int j = 0; j < 16; ++j)
		m_scalarParameterDataValid[i->second + j] = 1;
#endif

	m_dirty = true;
}

void ProgramWin32::setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length)
{
	SmallMap< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.find(handle);
	if (i == m_scalarParameterMap.end())
		return;

	for (int j = 0; j < length; ++j)
		param[j].storeAligned(&m_scalarParameterData[i->second + j * 16]);
#if defined(_DEBUG)
	for (int j = 0; j < length * 16; ++j)
		m_scalarParameterDataValid[i->second + j] = 1;
#endif

	m_dirty = true;
}

void ProgramWin32::setTextureParameter(handle_t handle, ITexture* texture)
{
	SmallMap< handle_t, uint32_t >::const_iterator i = m_textureParameterMap.find(handle);
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

#if defined(_DEBUG)
void ProgramWin32::validateParameter(const ProgramScalar& scalar)
{
	bool valid = (m_scalarParameterDataValid[scalar.offset] == 1);
	if (!valid)
	{
		std::wstring parameterName;
		for (SmallMap< handle_t, uint32_t >::const_iterator i = m_scalarParameterMap.begin(); i != m_scalarParameterMap.end(); ++i)
		{
			if (i->second == scalar.offset)
			{
				parameterName = m_scalarParameterNames[i->first];
				break;
			}
		}
		if (!parameterName.empty())
			log::error << L"Parameter \"" << parameterName << L"\" not set" << Endl;
		else
			log::error << L"Unknown parameter not set" << Endl;
	}
}
#endif

	}
}
