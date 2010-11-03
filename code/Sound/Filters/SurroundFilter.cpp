#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/ISoundMixer.h"
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
	float* m_buffer[SbcMaxChannelCount];

	SurroundFilterInstance()
	{
		const uint32_t bufferSize = 4096 * sizeof(float);
		for (int i = 0; i < sizeof_array(m_buffer); ++i)
		{
			m_buffer[i] = (float*)Alloc::acquireAlign(bufferSize, 16, T_FILE_LINE);
			std::memset(m_buffer[i], 0, bufferSize);
		}
	}

	virtual ~SurroundFilterInstance()
	{
		for (int i = 0; i < sizeof_array(m_buffer); ++i)
			Alloc::freeAlign(m_buffer[i]);
	}
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
	return new SurroundFilterInstance();
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

	float innerAtten = 1.0f - clamp(speakerDistance / innerRadius, 0.f, 1.0f);
	float distanceAtten = 1.0f - clamp(speakerDistance / maxDistance, 0.0f, 1.0f);

	const float c_angleCone = deg2rad(225.0f);

	for (uint32_t i = 0; i < sizeof_array(c_speakersStereo); ++i)
	{
		float* samples = outBlock.samples[c_speakersStereo[i].channel];
		T_ASSERT (alignUp(samples, 16) == samples);

		float angleOffset = angleDifference(c_speakersStereo[i].angle, speakerAngle);
		float angleAtten = clamp(1.0f - angleOffset / c_angleCone, 0.0f, 1.0f);
		Scalar attenuation = Scalar(innerAtten + (angleAtten * distanceAtten) * (1.0f - innerAtten));

		for (uint32_t j = 0; j < outBlock.samplesCount; j += 4)
		{
			Vector4 s4 = Vector4::loadAligned(&samples[j]);
			s4 *= attenuation;
			s4.storeAligned(&samples[j]);
		}
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
	const float maxDistance = m_environment->getMaxDistance();
	const float innerRadius = m_environment->getInnerRadius();

	Vector4 speakerPosition = listenerTransformInv * m_speakerPosition.xyz1();
	float speakerDistance = speakerPosition.xyz0().length();

	float distanceAtten = clamp(1.0f - speakerDistance / maxDistance, 0.0f, 1.0f);
	float innerAtten = clamp(speakerDistance / innerRadius, 0.0f, 1.0f);

	if (distanceAtten >= FUZZY_EPSILON)
	{
		float speakerAngle = angleRange(atan2f(-speakerPosition.z(), -speakerPosition.x()) + PI);
		const float c_angleCone = deg2rad(90.0f + 30.0f);

		for (uint32_t i = 0; i < sizeof_array(c_speakersFull); ++i)
		{
			float* samples = outBlock.samples[c_speakersFull[i].channel];
			T_ASSERT (alignUp(samples, 16) == samples);

			float angleOffset = angleDifference(c_speakersFull[i].angle, speakerAngle);
			float angleAtten = clamp(1.0f - angleOffset / c_angleCone, 0.0f, 1.0f);
			Scalar attenuation = Scalar(angleAtten * distanceAtten * (1.0f - c_speakersFull[i].inner * innerAtten));

			for (uint32_t j = 0; j < outBlock.samplesCount; j += 4)
			{
				Vector4 s4 = Vector4::loadAligned(&samples[j]);
				s4 *= attenuation;
				s4.storeAligned(&samples[j]);
			}
		}
	}
	else
	{
		for (uint32_t i = 0; i < sizeof_array(c_speakersFull); ++i)
		{
			float* samples = outBlock.samples[c_speakersFull[i].channel];
			T_ASSERT (alignUp(samples, 16) == samples);

			for (uint32_t j = 0; j < outBlock.samplesCount; j += 4)
				Vector4::zero().storeAligned(&samples[j]);
		}
	}
}

bool SurroundFilter::serialize(ISerializer& s)
{
	return s >> Member< Vector4 >(L"speakerPosition", m_speakerPosition);
}

	}
}
