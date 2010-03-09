#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/SoundBlockUtilities.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Sound/Filters/SurroundFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

float angleRange(float angle)
{
	while (angle < 0.0f)
		angle += TWO_PI;
	while (angle > TWO_PI)
		angle -= TWO_PI;
	return angle;
}

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

struct Speaker
{
	float angle;
	float inner;
	int channel;
};

const Speaker c_speakersStereo[] =
{
	{ deg2rad(0), 0.0f, SbcRight },
	{ deg2rad(180), 0.0f, SbcLeft }
};

const uint32_t c_speakersStereoMaxChannel = SbcRight + 1;

const Speaker c_speakersFull[] =
{
	{ deg2rad(45), 0.0f, SbcRight },
	{ deg2rad(135), 0.0f, SbcLeft },
	{ deg2rad(225), 0.0f, SbcRearLeft },
	{ deg2rad(315), 0.0f, SbcRearRight },
	{ deg2rad(90), 1.0f, SbcCenter },
	{ deg2rad(90), 1.0f, SbcLfe }
};

const uint32_t c_speakersFullMaxChannel = SbcRearRight + 1;

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
	if (m_environment->getFullSurround())
		applyFull(instance, outBlock);
	else
		applyStereo(instance, outBlock);
}

void SurroundFilter::applyStereo(IFilterInstance* instance, SoundBlock& outBlock) const
{
	SurroundFilterInstance* sfi = static_cast< SurroundFilterInstance* >(instance);

	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		float sample = 0.0f;
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
		{
			if (outBlock.samples[j])
				sample += outBlock.samples[j][i];
		}
		for (uint32_t j = 0; j < sizeof_array(c_speakersStereo); ++j)
			sfi->m_buffer[c_speakersStereo[j].channel][i] = sample;
	}

	outBlock.maxChannel = c_speakersStereoMaxChannel;
	for (uint32_t j = 0; j < c_speakersStereoMaxChannel; ++j)
		outBlock.samples[j] = sfi->m_buffer[j];

	const Transform& listenerTransformInv = m_environment->getListenerTransformInv();

	Vector4 speakerPosition = listenerTransformInv * m_speakerPosition.xyz1();
	float speakerDistance = speakerPosition.xyz0().length();
	float speakerAngle = angleRange(atan2f(-speakerPosition.z(), -speakerPosition.x()) + PI);

	float maxDistance = m_environment->getMaxDistance();
	float innerRadius = m_environment->getInnerRadius();

	float distanceAtten = clamp(1.0f - speakerDistance / maxDistance, 0.0f, 1.0f);
	float innerAtten = clamp(speakerDistance / innerRadius, 0.0f, 1.0f);

	const float c_angleCone = deg2rad(180.0f);

	for (uint32_t i = 0; i < sizeof_array(c_speakersStereo); ++i)
	{
		float angleOffset = angleDifference(c_speakersStereo[i].angle, speakerAngle);
		float angleAtten = clamp(1.0f - angleOffset / c_angleCone, 0.0f, 1.0f);
		float attenuation = angleAtten * distanceAtten * (1.0f - c_speakersFull[i].inner * innerAtten);
		soundBlockMulConst(outBlock.samples[c_speakersStereo[i].channel], outBlock.samplesCount, attenuation);
	}
}

void SurroundFilter::applyFull(IFilterInstance* instance, SoundBlock& outBlock) const
{
	SurroundFilterInstance* sfi = static_cast< SurroundFilterInstance* >(instance);

	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		float sample = 0.0f;
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
		{
			if (outBlock.samples[j])
				sample += outBlock.samples[j][i];
		}
		for (uint32_t j = 0; j < sizeof_array(c_speakersFull); ++j)
			sfi->m_buffer[c_speakersFull[j].channel][i] = sample;
	}

	outBlock.maxChannel = c_speakersFullMaxChannel;
	for (uint32_t j = 0; j < c_speakersFullMaxChannel; ++j)
		outBlock.samples[j] = sfi->m_buffer[j];

	const Transform& listenerTransformInv = m_environment->getListenerTransformInv();

	Vector4 speakerPosition = listenerTransformInv * m_speakerPosition.xyz1();
	float speakerDistance = speakerPosition.xyz0().length();
	float speakerAngle = angleRange(atan2f(-speakerPosition.z(), -speakerPosition.x()) + PI);

	float maxDistance = m_environment->getMaxDistance();
	float innerRadius = m_environment->getInnerRadius();

	float distanceAtten = clamp(1.0f - speakerDistance / maxDistance, 0.0f, 1.0f);
	float innerAtten = clamp(speakerDistance / innerRadius, 0.0f, 1.0f);

	const float c_angleCone = deg2rad(90.0f + 30.0f);

	for (uint32_t i = 0; i < sizeof_array(c_speakersFull); ++i)
	{
		float angleOffset = angleDifference(c_speakersFull[i].angle, speakerAngle);
		float angleAtten = clamp(1.0f - angleOffset / c_angleCone, 0.0f, 1.0f);
		float attenuation = angleAtten * distanceAtten * (1.0f - c_speakersFull[i].inner * innerAtten);
		soundBlockMulConst(outBlock.samples[c_speakersStereo[i].channel], outBlock.samplesCount, attenuation);
	}
}

bool SurroundFilter::serialize(ISerializer& s)
{
	return s >> Member< Vector4 >(L"speakerPosition", m_speakerPosition);
}

	}
}
