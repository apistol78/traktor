#ifndef traktor_illuminate_JobTraceDirect_H
#define traktor_illuminate_JobTraceDirect_H

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
		int32_t tileX,
		int32_t tileY,
		const SahTree& sah,
		const GBuffer& gbuffer,
		const AlignedVector< Light >& lights,
		drawing::Image* outputImageDirect,
		float pointLightRadius,
		int32_t shadowSamples,
		int32_t probeSamples,
		float probeCoeff,
		float probeSpread,
		float probeShadowSpread
	);

	void execute();

private:
	int32_t m_tileX;
	int32_t m_tileY;
	const SahTree& m_sah;
	const GBuffer& m_gbuffer;
	const AlignedVector< Light >& m_lights;
	drawing::Image* m_outputImageDirect;
	float m_pointLightRadius;
	int32_t m_shadowSamples;
	int32_t m_probeSamples;
	float m_probeCoeff;
	float m_probeSpread;
	float m_probeShadowSpread;
};

	}
}

#endif	// traktor_illuminate_JobTraceDirect_H
