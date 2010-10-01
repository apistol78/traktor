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
		VertexAttributeCount = 16,
		SamplerCount = 8
	};

	enum ResetFlags
	{
		RfVertexConstants = 1,
		RfRenderState = 2,
		RfSamplerStates = 4,
		RfForced = 8
	};

	StateCachePs3();

	~StateCachePs3();

	void setInFp32Mode(bool inFp32Mode);

	void setRenderState(const RenderState& rs);

	void setSamplerState(int32_t stage, const SamplerState& ss);

	void setSamplerTexture(int32_t stage, const CellGcmTexture* texture, uint16_t maxLod, uint8_t anisotropy);

	void setProgram(
		const CGprogram vertexProgram,
		const void* vertexUCode,
		const CGprogram fragmentProgram,
		const uint32_t fragmentOffset,
		bool updateFragmentConstants,
		bool updateFragmentTextures
	);

	void setVertexShaderConstant(uint32_t registerOffset, uint32_t registerCount, const float* constantData);

	void setViewport(const Viewport& viewport);

	void setVertexDataArray(uint8_t index, uint8_t stride, uint8_t size, uint8_t type, uint8_t location, uint32_t offset);

	void reset(uint32_t flags);

private:
	struct VertexAttribute
	{
		uint8_t stride;
		uint8_t size;
		uint8_t type;
		uint8_t location;
		uint32_t offset;
	};

	bool m_inFp32Mode;
	RenderState m_renderState;
	SamplerState m_samplerStates[SamplerCount];
	const void* m_vertexUCode;
	uint32_t m_fragmentOffset;
	float* m_vertexConstantsShadow;
	const CellGcmTexture* m_textures[SamplerCount];
	uint32_t m_textureOffsets[SamplerCount];
	uint16_t m_textureLods[SamplerCount];
	uint8_t m_textureAnisotropy[SamplerCount];
	uint32_t m_colorMask;
	Viewport m_viewport;
	VertexAttribute m_vertexAttributes[VertexAttributeCount];
};

	}
}

#endif	// traktor_render_StateCachePs3_H
