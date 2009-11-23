#ifndef traktor_render_ProgramDx10_H
#define traktor_render_ProgramDx10_H

#include <map>
#include "Render/IProgram.h"
#include "Render/Types.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX10_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx10;
class ProgramResourceDx10;
class HlslProgram;

/*!
 * \ingroup DX10
 */
class T_DLLCLASS ProgramDx10 : public IProgram
{
	T_RTTI_CLASS;

public:
	ProgramDx10(ContextDx10* context);

	virtual ~ProgramDx10();

	static Ref< ProgramResourceDx10 > compile(ID3D10Device* d3dDevice, const HlslProgram& hlslProgram);

	bool create(ID3D10Device* d3dDevice, const ProgramResourceDx10* resource);

	virtual void destroy();

	virtual void setFloatParameter(handle_t handle, float param);

	virtual void setFloatArrayParameter(handle_t handle, const float* param, int length);
	
	virtual void setVectorParameter(handle_t handle, const Vector4& param);

	virtual void setVectorArrayParameter(handle_t handle, const Vector4* param, int length);

	virtual void setMatrixParameter(handle_t handle, const Matrix44& param);

	virtual void setMatrixArrayParameter(handle_t handle, const Matrix44* param, int length);

	virtual void setSamplerTexture(handle_t handle, ITexture* texture);

	virtual void setStencilReference(uint32_t stencilReference);

	virtual bool isOpaque() const;

	bool bind(
		ID3D10Device* d3dDevice,
		size_t d3dInputElementsHash,
		const std::vector< D3D10_INPUT_ELEMENT_DESC >& d3dInputElements
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
		ComRef< ID3D10Buffer > d3dConstantBuffer;
		ComRefArray< ID3D10SamplerState > d3dSamplerStates;
		std::vector< ParameterOffset > parameterFloatOffsets;
		std::vector< std::pair< UINT, uint32_t > > resourceIndices;
	};

	static ProgramDx10* ms_activeProgram;
	Ref< ContextDx10 > m_context;
	ComRef< ID3D10RasterizerState > m_d3dRasterizerState;
	ComRef< ID3D10DepthStencilState > m_d3dDepthStencilState;
	ComRef< ID3D10BlendState > m_d3dBlendState;
	bool m_blendEnable;
	uint32_t m_stencilReference;
	ComRef< ID3D10VertexShader > m_d3dVertexShader;
	ComRef< ID3D10PixelShader > m_d3dPixelShader;
	State m_vertexState;
	State m_pixelState;
	ComRef< ID3D10Blob > m_d3dVertexShaderBlob;
	std::map< size_t, ComRef< ID3D10InputLayout > > m_d3dInputLayouts;
	ComRef< ID3D10InputLayout > m_d3dInputLayout;
	size_t m_d3dInputElementsHash;
	std::map< handle_t, uint32_t > m_parameterMap;
	std::vector< float > m_parameterFloatArray;
	ComRefArray< ID3D10ShaderResourceView > m_parameterResArray;
	bool m_parameterArrayDirty;
	bool m_parameterResArrayDirty;

	bool createState(
		ID3D10Device* d3dDevice,
		ID3D10Blob* d3dShaderBlob,
		const std::map< std::wstring, D3D10_SAMPLER_DESC >& d3dSamplers,
		State& outState
	);

	bool updateStateConstants(State& state);
};

	}
}

#endif	// traktor_render_ProgramDx10_H
