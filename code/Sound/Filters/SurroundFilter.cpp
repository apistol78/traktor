#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Sound/Filters/SurroundFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

float angleDifference(float angle1, float angle2)
{
	float A = abs(angle1 - angle2);
	float B = abs(angle1 + TWO_PI - angle2);
	float C = abs(angle2 + TWO_PI - angle1);
	return min(min(A, B), C);
}

struct SurroundFilterInstance : public RefCountImpl< IFilterInstance >
{
	float m_buffer[SbcMaxChannelCount][4096];
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SurroundFilter", SurroundFilter, IFilter)

SurroundFilter::SurroundFilter(SurroundEnvironment* environment)
:	m_environment(environment)
,	m_speakerPosition(0.0f, 0.0f, 0.0f, 1.0f)
{
}

void SurroundFilter::setSpeakerPosition(const Vector4& position)
{
	m_speakerPosition = position;
}

Ref< IFilterInstance > SurroundFilter::createInstance() const
{
	Ref< SurroundFilterInstance > sfi = new SurroundFilterInstance();
	std::memset(sfi->m_buffer, 0, sizeof(sfi->m_buffer));
	return sfi;
}

void SurroundFilter::apply(IFilterInstance* instance, SoundBlock& outBlock) const
{
	SurroundFilterInstance* sfi = static_cast< SurroundFilterInstance* >(instance);

	const struct Speaker
	{
		float angle;
		int channel;
	}
	c_speakers[] =
	{
		{ deg2rad(45), SbcRight },
		{ deg2rad(135), SbcLeft },
		{ deg2rad(225), SbcRearLeft },
		{ deg2rad(315), SbcRearRight }
	};

	// Collapse all channels; should be mono sound sources.
	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		float sample = 0.0f;
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
			sample += outBlock.samples[j][i];
		for (uint32_t j = 0; j < sizeof_array(c_speakers); ++j)
			sfi->m_buffer[c_speakers[j].channel][i] = sample;
		sfi->m_buffer[SbcCenter][i] = sample;
		sfi->m_buffer[SbcLfe][i] = 0.0f;
	}

	outBlock.maxChannel = SbcMaxChannelCount;
	for (uint32_t j = 0; j < SbcMaxChannelCount; ++j)
		outBlock.samples[j] = sfi->m_buffer[j];

	// Get speaker position in listener space.
	Matrix44 listenerTransformInv = m_environment->getListenerTransformInv();

	Vector4 speakerPosition = listenerTransformInv * m_speakerPosition.xyz1();
	float speakerDistance = speakerPosition.xyz0().length();
	float speakerAngle = atan2f(speakerPosition.x(), speakerPosition.z()) + PI;

	float maxDistance = m_environment->getMaxDistance();
	float innerRadius = m_environment->getInnerRadius();

	float distanceAtten = clamp(1.0f - speakerDistance / maxDistance, 0.0f, 1.0f);
	float innerAtten = clamp(speakerDistance / innerRadius, 0.0f, 1.0f);

	const float c_angleCone = deg2rad(90.0f + 30.0f);

	// Satellite speakers.
	for (uint32_t i = 0; i < sizeof_array(c_speakers); ++i)
	{
		float angleOffset = angleDifference(c_speakers[i].angle, speakerAngle);
		float angleAtten = clamp(1.0f - angleOffset / c_angleCone, 0.0f, 1.0f);
		float attenuation = angleAtten * distanceAtten * innerAtten;

		for (uint32_t j = 0; j < outBlock.samplesCount; ++j)
			outBlock.samples[c_speakers[i].channel][j] *= attenuation;
	}

	// Center speaker.
	for (uint32_t j = 0; j < outBlock.samplesCount; ++j)
	{
		outBlock.samples[SbcCenter][j] *= (1.0f - innerAtten);
		outBlock.samples[SbcLfe][j] = outBlock.samples[SbcCenter][j];
	}
}

bool SurroundFilter::serialize(ISerializer& s)
{
	return s >> Member< Vector4 >(L"speakerPosition", m_speakerPosition);
}

	}
}
