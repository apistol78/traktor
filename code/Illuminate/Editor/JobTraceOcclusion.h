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

class JobTraceOcclusion
{
public:
	JobTraceOcclusion(
		const SahTree& sah,
		const GBuffer& gbuffer,
		drawing::Image* outputImageOcclusion,
		int32_t occlusionSamples
	);

	void execute(int32_t tileX, int32_t tileY) const;

private:
	const SahTree& m_sah;
	const GBuffer& m_gbuffer;
	drawing::Image* m_outputImageOcclusion;
	int32_t m_occlusionSamples;
};

	}
}
