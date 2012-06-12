#ifndef traktor_render_ProgramDx11_H
#define traktor_render_ProgramDx11_H

#include <map>
#include "Core/Containers/SmallMap.h"
#include "Core/Misc/ComRef.h"
#include "Render/IProgram.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx11;
class ProgramResourceDx11;
class StateCache;
class HlslProgram;

/*!
 * \ingroup DX11
 */
class T_DLLCLASS ProgramDx11 : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramDx11(ContextDx11* context);

	virtual ~ProgramDx11();

	bool create(ID3D11Device* d3dDevice, StateCache& stateCache, const ProgramResourceDx11* resource, float mipBias, int32_t maxAnisotropy);

	virtual void destroy();

	virtual void setFloatParameter(handle_t handle, float param);

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length);
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param);

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param);

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	virtual void setTextureParameter(handle_t handle, ITexture* texture);

	virtual void setStencilReference(uint32_t stencilReference);

	bool bind(
		ID3D11Device* d3dDevice,
		ID3D11DeviceContext* d3dDeviceContext,
		size_t d3dInputElementsHash,
		const std::vector< D3D11_INPUT_ELEMENT_DESC >& d3dInputElements,
		const int32_t targetSize[2]
	);

	void unbind(
		ID3D11Device* d3dDevice,
		ID3D11DeviceContext* d3dDeviceContext
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

	struct State
	{
		ComRef< ID3D11Buffer > d3dConstantBuffer[4];
		ComRefArray< ID3D11SamplerState > d3dSamplerStates;
		std::vector< ParameterOffset > parameterFloatOffsets;
		std::vector< std::pair< UINT, uint32_t > > resourceIndices;
	};

	static ProgramDx11* ms_activeProgram;
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11RasterizerState > m_d3dRasterizerState;
	ComRef< ID3D11DepthStencilState > m_d3dDepthStencilState;
	ComRef< ID3D11BlendState > m_d3dBlendState;
	uint32_t m_stencilReference;
	ComRef< ID3D11VertexShader > m_d3dVertexShader;
	ComRef< ID3D11PixelShader > m_d3dPixelShader;
	State m_vertexState;
	State m_pixelState;
	ComRef< ID3DBlob > m_d3dVertexShaderBlob;
	SmallMap< size_t, ComRef< ID3D11InputLayout > > m_d3dInputLayouts;
	ComRef< ID3D11InputLayout > m_d3dInputLayout;
	size_t m_d3dInputElementsHash;
	SmallMap< handle_t, uint32_t > m_parameterMap;
	AlignedVector< float > m_parameterFloatArray;
	ComRefArray< ID3D11ShaderResourceView > m_parameterResArray;
	bool m_parameterArrayDirty;
	bool m_parameterResArrayDirty;
	uint32_t m_bufferCycle;

	bool createState(
		ID3D11Device* d3dDevice,
		float mipBias,
		int32_t maxAnisotropy,
		ID3DBlob* d3dShaderBlob,
		const std::map< std::wstring, D3D11_SAMPLER_DESC >& d3dSamplers,
		State& outState
	);

	bool updateStateConstants(ID3D11DeviceContext* d3dDeviceContext, State& state);
};

	}
}

#endif	// traktor_render_ProgramDx11_H
