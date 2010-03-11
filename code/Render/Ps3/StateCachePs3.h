#ifndef traktor_render_StateCachePs3_H
#define traktor_render_StateCachePs3_H

#include "Render/Ps3/TypesPs3.h"

namespace traktor
{
	namespace render
	{

class ITexture;

class StateCachePs3
{
public:
	enum
	{
		VertexConstantCount = 256,
		SamplerCount = 8
	};

	enum ResetFlags
	{
		RfRenderState = 1,
		RfSamplerStates = 2,
		RfForced = 4
	};

	StateCachePs3();

	~StateCachePs3();

	void setInFp32Mode(bool inFp32Mode);

	void setRenderState(const RenderState& rs);

	void setSamplerState(int32_t stage, const SamplerState& ss);

	void setSamplerTexture(int32_t stage, const CellGcmTexture* texture, uint16_t maxLod);

	void setProgram(const CGprogram vertexProgram, const void* vertexUCode, const CGprogram fragmentProgram, const uint32_t fragmentOffset, bool updateFragmentProgram);

	void setVertexShaderConstant(uint32_t registerOffset, uint32_t registerCount, const float* constantData);

	void setColorMask(uint32_t colorMask);

	void setViewport(const Viewport& viewport);

	void reset(uint32_t flags);

private:
	bool m_inFp32Mode;
	RenderState m_renderState;
	SamplerState m_samplerStates[SamplerCount];
	const void* m_vertexUCode;
	uint32_t m_fragmentOffset;
	float* m_vertexConstantsShadow;
	const CellGcmTexture* m_textures[SamplerCount];
	uint32_t m_textureOffsets[SamplerCount];
	uint16_t m_textureLods[SamplerCount];
	uint32_t m_colorMask;
	Viewport m_viewport;
};

	}
}

#endif	// traktor_render_StateCachePs3_H
