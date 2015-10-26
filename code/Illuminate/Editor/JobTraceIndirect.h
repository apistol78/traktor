#ifndef traktor_illuminate_JobTraceIndirect_H
#define traktor_illuminate_JobTraceIndirect_H

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

class JobTraceIndirect
{
public:
	JobTraceIndirect(
		int32_t tileX,
		int32_t tileY,
		const SahTree& sah,
		const GBuffer& gbuffer,
		const AlignedVector< Surface >& surfaces,
		const drawing::Image* imageIrradiance,
		drawing::Image* outputImageIndirect,
		int32_t indirectTraceSamples
	);

	void execute();

private:
	int32_t m_tileX;
	int32_t m_tileY;
	const SahTree& m_sah;
	const GBuffer& m_gbuffer;
	const AlignedVector< Surface >& m_surfaces;
	const drawing::Image* m_imageIrradiance;
	drawing::Image* m_outputImageIndirect;
	int32_t m_indirectTraceSamples;
};

	}
}

#endif	// traktor_illuminate_JobTraceIndirect_H
