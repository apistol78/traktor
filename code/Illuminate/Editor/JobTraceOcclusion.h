#ifndef traktor_illuminate_JobTraceOcclusion_H
#define traktor_illuminate_JobTraceOcclusion_H

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
		int32_t tileX,
		int32_t tileY,
		const SahTree& sah,
		const GBuffer& gbuffer,
		drawing::Image* outputImageOcclusion,
		int32_t occlusionSamples
	);

	void execute();

private:
	int32_t m_tileX;
	int32_t m_tileY;
	const SahTree& m_sah;
	const GBuffer& m_gbuffer;
	drawing::Image* m_outputImageOcclusion;
	int32_t m_occlusionSamples;
};

	}
}

#endif	// traktor_illuminate_JobTraceOcclusion_H
