#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Illuminate/Editor/Types.h"

namespace traktor
{

class SahTree;

	namespace drawing
	{

class Image;

	}

	namespace illuminate
	{

class GBuffer;

class JobTraceDirect
{
public:
	JobTraceDirect(
		const SahTree& sah,
		const GBuffer& gbuffer,
		const AlignedVector< Light >& lights,
		drawing::Image* outputImageDirect,
		const drawing::Image* imageOcclusion,
		float pointLightRadius,
		int32_t shadowSamples
	);

	void execute(int32_t tileX, int32_t tileY) const;

private:
	const SahTree& m_sah;
	const GBuffer& m_gbuffer;
	const AlignedVector< Light >& m_lights;
	drawing::Image* m_outputImageDirect;
	const drawing::Image* m_imageOcclusion;
	float m_pointLightRadius;
	int32_t m_shadowSamples;
};

	}
}
