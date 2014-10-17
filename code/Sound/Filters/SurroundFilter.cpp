#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Memory/Alloc.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/Align.h"
#include "Core/Serialization/AttributePoint.h"
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

Scalar angleRange(const Scalar& _angle)
{
	const Scalar twoPi(TWO_PI);
	Scalar angle = _angle;
	while (angle < 0.0f)
		angle += twoPi;
	while (angle > TWO_PI)
		angle -= twoPi;
	return angle;
}

Scalar angleDifference(const Scalar& angle1, const Scalar& angle2)
{
	const Scalar twoPi(TWO_PI);
	Scalar A = abs(angle1 - angle2);
	Scalar B = abs(angle1 + twoPi - angle2);
	Scalar C = abs(angle2 + twoPi - angle1);
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

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

struct Speaker
{
	Scalar angle;
	Scalar inner;
	int channel;
};

const Speaker c_speakersStereo[] =
{
	{ Scalar(deg2rad(0)), Scalar(0.0f), SbcRight },
	{ Scalar(deg2rad(180)), Scalar(0.0f), SbcLeft }
};

const uint32_t c_speakersStereoMaxChannel = SbcRight + 1;

const Speaker c_speakersFull[] =
{
#if T_SOUND_SPEAKERS_SETUP == T_SOUND_SPEAKER_SETUP_2_0
	{ Scalar(deg2rad(0)), Scalar(0.0f), SbcRight },
	{ Scalar(deg2rad(180)), Scalar(0.0f), SbcLeft }
#elif T_SOUND_SPEAKERS_SETUP == T_SOUND_SPEAKER_SETUP_5_1
	{ Scalar(deg2rad(45)), Scalar(0.0f), SbcRight },
	{ Scalar(deg2rad(135)), Scalar(0.0f), SbcLeft },
	{ Scalar(deg2rad(225)), Scalar(0.0f), SbcRearLeft },
	{ Scalar(deg2rad(315)), Scalar(0.0f), SbcRearRight },
	{ Scalar(deg2rad(90)), Scalar(1.0f), SbcCenter },
	{ Scalar(deg2rad(90)), Scalar(1.0f), SbcLfe }
#elif T_SOUND_SPEAKERS_SETUP == T_SOUND_SPEAKER_SETUP_7_1
	{ Scalar(deg2rad(45)), Scalar(0.0f), SbcRight },
	{ Scalar(deg2rad(135)), Scalar(0.0f), SbcLeft },
	{ Scalar(deg2rad(225)), Scalar(0.0f), SbcRearLeft },
	{ Scalar(deg2rad(315)), Scalar(0.0f), SbcRearRight },
	{ Scalar(deg2rad(90)), Scalar(1.0f), SbcCenter },
	{ Scalar(deg2rad(90)), Scalar(1.0f), SbcLfe }
#endif
};

#if T_SOUND_SPEAKERS_SETUP == T_SOUND_SPEAKER_SETUP_2_0
const uint32_t c_speakersFullMaxChannel = SbcRight + 1;
#elif T_SOUND_SPEAKERS_SETUP == T_SOUND_SPEAKER_SETUP_5_1
const uint32_t c_speakersFullMaxChannel = SbcRearRight + 1;
#elif T_SOUND_SPEAKERS_SETUP == T_SOUND_SPEAKER_SETUP_7_1
const uint32_t c_speakersFullMaxChannel = SbcRearRight + 1;
#endif

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SurroundFilter", SurroundFilter, IFilter)

Ref< SurroundFilter > SurroundFilter::create(SurroundEnvironment* environment, const Vector4& speakerPosition)
{
	if (!environment)
		return 0;

	const Transform& listenerTransformInv = environment->getListenerTransformInv();

	Vector4 speakerPositionL = listenerTransformInv * speakerPosition.xyz1();
	Scalar speakerDistance = speakerPositionL.xyz0().length();

	if (speakerDistance >= environment->getMaxDistance())
		return 0;

	return new SurroundFilter(environment, speakerPosition);
}

SurroundFilter::SurroundFilter(SurroundEnvironment* environment, const Vector4& speakerPosition, float maxDistance)
:	m_environment(environment)
,	m_speakerPosition(speakerPosition)
,	m_maxDistance(maxDistance)
{
	if (m_maxDistance <= m_environment->getInnerRadius())
		m_maxDistance = m_environment->getMaxDistance();
}

void SurroundFilter::setSpeakerPosition(const Vector4& speakerPosition)
{
	m_speakerPosition = speakerPosition;
}

void SurroundFilter::setMaxDistance(float maxDistance)
{
	m_maxDistance = Scalar(maxDistance);
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

	const Scalar c_angleCone(deg2rad(225.0f));
	const Scalar c_one(1.0f);
	const Scalar c_zero(0.0f);

	// Combine all channels into a mono channel.
	for (uint32_t i = 0; i < outBlock.samplesCount; i += 4)
	{
		Vector4 s4 = Vector4::zero();
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
		{
			if (outBlock.samples[j])
				s4 += Vector4::loadAligned(&outBlock.samples[j][i]);
		}
		for (uint32_t j = 0; j < sizeof_array(c_speakersStereo); ++j)
			s4.storeAligned(&sfi->m_buffer[c_speakersStereo[j].channel][i]);
	}

	const Transform& listenerTransformInv = m_environment->getListenerTransformInv();

	Vector4 speakerPosition = listenerTransformInv * m_speakerPosition.xyz1();
	Scalar speakerDistance = speakerPosition.xyz0().length();
	Scalar speakerAngle = angleRange(Scalar(atan2f(-speakerPosition.z(), -speakerPosition.x()) + PI));

	const Scalar& innerRadius = m_environment->getInnerRadius();

	Scalar innerAtten = c_one - clamp(squareRoot(speakerDistance / innerRadius), c_zero, c_one);
	Scalar distanceAtten = c_one - clamp(squareRoot(speakerDistance / m_maxDistance), c_zero, c_one);

	for (uint32_t i = 0; i < sizeof_array(c_speakersStereo); ++i)
	{
		float* inputSamples = outBlock.samples[c_speakersStereo[i].channel];
		float* directionalSamples = sfi->m_buffer[c_speakersStereo[i].channel];

		Scalar angleOffset = angleDifference(c_speakersStereo[i].angle, speakerAngle);
		Scalar angleAtten = clamp(c_one - angleOffset / c_angleCone, c_zero, c_one);
		Scalar directionalAtten = innerAtten + (angleAtten * distanceAtten) * (c_one - innerAtten);

		if (inputSamples)
		{
			for (uint32_t j = 0; j < outBlock.samplesCount; j += 4)
			{
				Vector4 si4 = Vector4::loadAligned(&inputSamples[j]);
				Vector4 sd4 = Vector4::loadAligned(&directionalSamples[j]);
				Vector4 s4 = sd4 * directionalAtten + si4 * innerAtten;
				s4.storeAligned(&directionalSamples[j]);
			}
		}
		else
		{
			for (uint32_t j = 0; j < outBlock.samplesCount; j += 4)
			{
				Vector4 sd4 = Vector4::loadAligned(&directionalSamples[j]);
				Vector4 s4 = sd4 * directionalAtten;
				s4.storeAligned(&directionalSamples[j]);
			}
		}

		outBlock.samples[c_speakersStereo[i].channel] = directionalSamples;
	}

	outBlock.maxChannel = c_speakersStereoMaxChannel;
}

void SurroundFilter::applyFull(IFilterInstance* instance, SoundBlock& outBlock) const
{
	SurroundFilterInstance* sfi = static_cast< SurroundFilterInstance* >(instance);

	const Scalar c_angleCone(deg2rad(90.0f + 30.0f));
	const Scalar c_one(1.0f);
	const Scalar c_zero(0.0f);

	for (uint32_t i = 0; i < outBlock.samplesCount; i += 4)
	{
		Vector4 s4 = Vector4::zero();
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
		{
			if (outBlock.samples[j])
				s4 += Vector4::loadAligned(&outBlock.samples[j][i]);
		}
		for (uint32_t j = 0; j < sizeof_array(c_speakersFull); ++j)
			s4.storeAligned(&sfi->m_buffer[c_speakersFull[j].channel][i]);
	}

	outBlock.maxChannel = c_speakersFullMaxChannel;
	for (uint32_t j = 0; j < c_speakersFullMaxChannel; ++j)
		outBlock.samples[j] = sfi->m_buffer[j];

	const Transform& listenerTransformInv = m_environment->getListenerTransformInv();

	Vector4 speakerPosition = listenerTransformInv * m_speakerPosition.xyz1();
	Scalar speakerDistance = speakerPosition.xyz0().length();

	const Scalar& innerRadius = m_environment->getInnerRadius();

	Scalar distanceAtten = c_one - clamp(squareRoot(speakerDistance / m_maxDistance), c_zero, c_one);
	Scalar innerAtten = clamp(squareRoot(speakerDistance / innerRadius), c_zero, c_one);

	if (distanceAtten >= FUZZY_EPSILON)
	{
		Scalar speakerAngle = angleRange(Scalar(atan2f(-speakerPosition.z(), -speakerPosition.x()) + PI));

		for (uint32_t i = 0; i < sizeof_array(c_speakersFull); ++i)
		{
			float* samples = outBlock.samples[c_speakersFull[i].channel];
			T_ASSERT (alignUp(samples, 16) == samples);

			Scalar angleOffset = angleDifference(c_speakersFull[i].angle, speakerAngle);
			Scalar angleAtten = clamp(c_one - angleOffset / c_angleCone, c_zero, c_one);
			Scalar attenuation = angleAtten * distanceAtten * (c_one - c_speakersFull[i].inner * innerAtten);

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

void SurroundFilter::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"speakerPosition", m_speakerPosition, AttributePoint());
	s >> Member< Scalar >(L"maxDistance", m_maxDistance);
}

	}
}
