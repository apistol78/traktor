/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramResourceDx11_H
#define traktor_render_ProgramResourceDx11_H

#include <map>
#include <vector>
#include "Render/Dx11/Platform.h"
#include "Render/Resource/ProgramResource.h"

namespace traktor
{
	namespace render
	{

class Blob;

/*!
 * \ingroup DX11
 */
class ProgramResourceDx11 : public ProgramResource
{
	T_RTTI_CLASS;

public:
	struct ParameterDesc
	{
		std::wstring name;
		uint32_t offset;
		uint32_t count;

		ParameterDesc()
		:	offset(0)
		,	count(0)
		{
		}

		void serialize(ISerializer& s);
	};

	struct ParameterMappingDesc
	{
		uint32_t cbufferOffset;
		uint32_t parameterOffset;
		uint32_t parameterCount;

		ParameterMappingDesc()
		:	cbufferOffset(0)
		,	parameterOffset(0)
		,	parameterCount(0)
		{
		}

		ParameterMappingDesc(uint32_t cbufferOffset_, uint32_t parameterOffset_, uint32_t parameterCount_)
		:	cbufferOffset(cbufferOffset_)
		,	parameterOffset(parameterOffset_)
		,	parameterCount(parameterCount_)
		{
		}

		void serialize(ISerializer& s);
	};

	struct CBufferDesc
	{
		uint32_t size;
		std::vector< ParameterMappingDesc > parameters;

		CBufferDesc()
		:	size(0)
		{
		}

		void serialize(ISerializer& s);
	};

	struct TextureBindingDesc
	{
		uint32_t bindPoint;
		uint32_t parameterOffset;

		TextureBindingDesc()
		:	bindPoint(0)
		,	parameterOffset(0)
		{
		}

		TextureBindingDesc(uint32_t bindPoint_, uint32_t parameterOffset_)
		:	bindPoint(bindPoint_)
		,	parameterOffset(parameterOffset_)
		{
		}

		void serialize(ISerializer& s);
	};

	ProgramResourceDx11();

	virtual void serialize(ISerializer& s);

private:
	friend class ProgramDx11;
	friend class ProgramCompilerDx11;

	Ref< Blob > m_vertexShader;
	Ref< Blob > m_pixelShader;
	
	uint32_t m_vertexShaderHash;
	uint32_t m_pixelShaderHash;

	CBufferDesc m_vertexCBuffers[3];
	CBufferDesc m_pixelCBuffers[3];

	std::vector< TextureBindingDesc > m_vertexTextureBindings;
	std::vector< TextureBindingDesc > m_pixelTextureBindings;

	std::vector< D3D11_SAMPLER_DESC > m_vertexSamplers;
	std::vector< D3D11_SAMPLER_DESC > m_pixelSamplers;

	std::vector< ParameterDesc > m_parameters;
	uint32_t m_parameterScalarSize;
	uint32_t m_parameterTextureSize;

	D3D11_RASTERIZER_DESC m_d3dRasterizerDesc;
	D3D11_DEPTH_STENCIL_DESC m_d3dDepthStencilDesc;
	D3D11_BLEND_DESC m_d3dBlendDesc;
	uint32_t m_stencilReference;
};

	}
}

#endif	// traktor_render_ProgramResourceDx11_H
