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
		MaxTextureCount = 8
	};

	StateCachePs3();

	~StateCachePs3();

	void setInFp32Mode(bool inFp32Mode);

	void setRenderState(const RenderState& rs);

	void setProgram(const CGprogram vertexProgram, const void* vertexUCode, const CGprogram fragmentProgram, const uint32_t fragmentOffset, bool updateFragmentProgram);

	void setVertexShaderConstant(uint32_t registerOffset, uint32_t registerCount, const float* constantData);

	void setTexture(uint16_t stage, ITexture* texture, const SamplerState& samplerState);

	void reset(bool force);

	void resetTextures();

private:
	bool m_inFp32Mode;
	RenderState m_renderState;
	const void* m_vertexUCode;
	uint32_t m_fragmentOffset;
	float* m_vertexConstantsShadow;
	ITexture* m_textures[MaxTextureCount];
};

	}
}

#endif	// traktor_render_StateCachePs3_H
