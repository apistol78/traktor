/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ProgramDx11_H
#define traktor_render_ProgramDx11_H

#include <map>
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Misc/ComRef.h"
#include "Render/IProgram.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

class Blob;
class ContextDx11;
class ProgramResourceDx11;
class ResourceCache;
class StateCache;
class HlslProgram;

/*!
 * \ingroup DX11
 */
class ProgramDx11 : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramDx11(ContextDx11* context);

	virtual ~ProgramDx11();

	bool create(ID3D11Device* d3dDevice, ResourceCache& resourceCache, const ProgramResourceDx11* resource);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setFloatParameter(handle_t handle, float param) T_OVERRIDE T_FINAL;

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length) T_OVERRIDE T_FINAL;
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param) T_OVERRIDE T_FINAL;

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length) T_OVERRIDE T_FINAL;

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param) T_OVERRIDE T_FINAL;

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length) T_OVERRIDE T_FINAL;

	virtual void setTextureParameter(handle_t handle, ITexture* texture) T_OVERRIDE T_FINAL;

	virtual void setStencilReference(uint32_t stencilReference) T_OVERRIDE T_FINAL;

	bool bind(
		ID3D11Device* d3dDevice,
		ID3D11DeviceContext* d3dDeviceContext,
		StateCache& stateCache,
		uint32_t d3dInputElementsHash,
		const std::vector< D3D11_INPUT_ELEMENT_DESC >& d3dInputElements,
		const int32_t targetSize[2]
	);

private:
	struct ParameterOffset
	{
		UINT constant;
		uint32_t offset;
		uint32_t count;

		ParameterOffset()
		:	constant(0)
		,	offset(0)
		,	count(0)
		{
		}

		ParameterOffset(UINT constant_, uint32_t offset_, uint32_t count_)
		:	constant(constant_)
		,	offset(offset_)
		,	count(count_)
		{
		}
	};

	struct CBuffer
	{
#if defined(_DEBUG)
		std::wstring name;
#endif
		ComRef< ID3D11Buffer > d3dBuffer;
		AlignedVector< ParameterOffset > parameterOffsets;
		ID3D11DeviceContext* d3dMappedContext;
		bool dirty;

		CBuffer()
		:	d3dMappedContext(0)
		,	dirty(true)
		{
		}
	};

	struct ParameterMap
	{
#if defined(_DEBUG)
		std::wstring name;
#endif
		uint32_t offset;
		uint32_t count;
		CBuffer* cbuffer[2];

		ParameterMap()
		:	offset(0)
		{
			cbuffer[0] =
			cbuffer[1] = 0;
		}
	};

	struct State
	{
		CBuffer cbuffer[3];
		ComRefArray< ID3D11SamplerState > d3dSamplerStates;
		AlignedVector< std::pair< UINT, uint32_t > > resourceIndices;
	};

	Ref< ContextDx11 > m_context;
	ComRef< ID3D11RasterizerState > m_d3dRasterizerState;
	ComRef< ID3D11DepthStencilState > m_d3dDepthStencilState;
	ComRef< ID3D11BlendState > m_d3dBlendState;
	uint32_t m_stencilReference;
	ComRef< ID3D11VertexShader > m_d3dVertexShader;
	ComRef< ID3D11PixelShader > m_d3dPixelShader;
	State m_vertexState;
	State m_pixelState;
	Ref< Blob > m_d3dVertexShaderBlob;
	uint32_t m_d3dVertexShaderHash;
	SmallMap< handle_t, ParameterMap > m_parameterMap;
	AlignedVector< float > m_parameterFloatArray;
	RefArray< ITexture > m_parameterTextureArray;
	bool m_parameterTextureArrayDirty;

#if defined(_DEBUG)
	int32_t m_bindCount;
#endif

	bool updateStateConstants(ID3D11DeviceContext* d3dDeviceContext, State& state);
};

	}
}

#endif	// traktor_render_ProgramDx11_H
