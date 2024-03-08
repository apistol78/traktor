/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
#include "Sound/IAudioMixer.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Sound/Filters/SurroundFilter.h"

namespace traktor::sound
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
	const Scalar A = abs(angle1 - angle2);
	const Scalar B = abs(angle1 + twoPi - angle2);
	const Scalar C = abs(angle2 + twoPi - angle1);
	return min(min(A, B), C);
}

struct SurroundFilterInstance : public RefCountImpl< IAudioFilterInstance >
{
	float* m_mono;
	float* m_buffer[SbcMaxChannelCount];

	SurroundFilterInstance()
	{
		const uint32_t bufferSize = 4096 * sizeof(float);

		m_mono = (float*)Alloc::acquireAlign(bufferSize, 16, T_FILE_LINE);

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

		Alloc::freeAlign(m_mono);
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SurroundFilter", SurroundFilter, IAudioFilter)

SurroundFilter::SurroundFilter(SurroundEnvironment* environment, const Vector4& speakerPosition, float maxDistance)
:	m_environment(environment)
,	m_speakerPosition(speakerPosition)
,	m_maxDistance(maxDistance)
{
}

void SurroundFilter::setSpeakerPosition(const Vector4& speakerPosition)
{
	m_speakerPosition = speakerPosition;
}

void SurroundFilter::setMaxDistance(float maxDistance)
{
	m_maxDistance = Scalar(maxDistance);
}

Ref< IAudioFilterInstance > SurroundFilter::createInstance() const
{
	return new SurroundFilterInstance();
}

void SurroundFilter::apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const
{
	if (m_environment->getFullSurround())
		applyFull(instance, outBlock);
	else
		applyStereo(instance, outBlock);
}

void SurroundFilter::applyStereo(IAudioFilterInstance* instance, AudioBlock& outBlock) const
{
	SurroundFilterInstance* sfi = static_cast< SurroundFilterInstance* >(instance);

	const Scalar c_angleCone(deg2rad(225.0f));

	// Combine all channels into a mono channel.
	for (uint32_t i = 0; i < outBlock.samplesCount; i += 4)
	{
		Vector4 s4 = Vector4::zero();
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
		{
			if (outBlock.samples[j])
				s4 += Vector4::loadAligned(&outBlock.samples[j][i]);
		}
		s4.storeAligned(&sfi->m_mono[i]);
	}

	// Prepare output blocks.
	for (uint32_t i = 0; i < sizeof_array(c_speakersStereo); ++i)
	{
		float* outputSamples = sfi->m_buffer[c_speakersStereo[i].channel];
		std::memset(outputSamples, 0, outBlock.samplesCount * sizeof(float));
		outBlock.samples[c_speakersStereo[i].channel] = outputSamples;
	}
	outBlock.maxChannel = c_speakersStereoMaxChannel;

	// Mix in all listeners.
	for (auto& listenerTransform : m_environment->getListenerTransforms())
	{
		const Transform listenerTransformInv = listenerTransform.inverse();

		const Vector4 speakerPosition = listenerTransformInv * m_speakerPosition.xyz1();
		const Scalar speakerDistance = speakerPosition.xyz0().length();
		const Scalar speakerAngle = angleRange(Scalar(atan2f(-speakerPosition.z(), -speakerPosition.x()) + PI));

		const Scalar& innerRadius = m_environment->getInnerRadius();
		const Scalar distanceAtten = power(clamp(1.0_simd - (speakerDistance - innerRadius) / m_maxDistance, 0.0_simd, 1.0_simd), 1.0_simd);

		for (uint32_t i = 0; i < sizeof_array(c_speakersStereo); ++i)
		{
			float* outputSamples = sfi->m_buffer[c_speakersStereo[i].channel];
			float* directionalSamples = sfi->m_mono;

			const Scalar angleOffset = angleDifference(c_speakersStereo[i].angle, speakerAngle);
			const Scalar angleAtten = clamp(1.0_simd - angleOffset / c_angleCone, 0.0_simd, 1.0_simd);
			const Scalar directionalAtten = (angleAtten * distanceAtten);

			for (uint32_t j = 0; j < outBlock.samplesCount; j += 4)
			{
				const Vector4 sd4 = Vector4::loadAligned(&directionalSamples[j]);
				const Vector4 s4 = sd4 * directionalAtten;
				(Vector4::loadAligned(&outputSamples[j]) + s4).storeAligned(&outputSamples[j]);
			}
		}
	}
}

void SurroundFilter::applyFull(IAudioFilterInstance* instance, AudioBlock& outBlock) const
{
	//SurroundFilterInstance* sfi = static_cast< SurroundFilterInstance* >(instance);

	//const Scalar c_angleCone(deg2rad(90.0f + 30.0f));

	//// Combine all channels into a mono channel.
	//for (uint32_t i = 0; i < outBlock.samplesCount; i += 4)
	//{
	//	Vector4 s4 = Vector4::zero();
	//	for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
	//	{
	//		if (outBlock.samples[j])
	//			s4 += Vector4::loadAligned(&outBlock.samples[j][i]);
	//	}
	//	s4.storeAligned(&sfi->m_mono[i]);
	//}

	//const Transform& listenerTransformInv = m_environment->getListenerTransformInv();

	//const Vector4 speakerPosition = listenerTransformInv * m_speakerPosition.xyz1();
	//const Scalar speakerDistance = speakerPosition.xyz0().length();

	//const Scalar& innerRadius = m_environment->getInnerRadius();

	//const Scalar distanceAtten = 1.0_simd - clamp(squareRoot(speakerDistance / m_maxDistance), 0.0_simd, 1.0_simd);
	//const Scalar innerAtten = clamp(squareRoot(speakerDistance / innerRadius), 0.0_simd, 1.0_simd);

	//if (distanceAtten >= FUZZY_EPSILON)
	//{
	//	const Scalar speakerAngle = angleRange(Scalar(atan2f(-speakerPosition.z(), -speakerPosition.x()) + PI));

	//	for (uint32_t i = 0; i < sizeof_array(c_speakersFull); ++i)
	//	{
	//		float* inputSamples = sfi->m_mono;
	//		float* outputSamples = sfi->m_buffer[c_speakersFull[i].channel];

	//		const Scalar angleOffset = angleDifference(c_speakersFull[i].angle, speakerAngle);
	//		const Scalar angleAtten = clamp(1.0_simd - angleOffset / c_angleCone, 0.0_simd, 1.0_simd);
	//		const Scalar attenuation = angleAtten * distanceAtten * (1.0_simd - c_speakersFull[i].inner * innerAtten);

	//		for (uint32_t j = 0; j < outBlock.samplesCount; j += 4)
	//		{
	//			Vector4 s4 = Vector4::loadAligned(&inputSamples[j]);
	//			s4 *= attenuation;
	//			s4.storeAligned(&outputSamples[j]);
	//		}

	//		outBlock.samples[c_speakersFull[i].channel] = outputSamples;
	//	}
	//}
	//else
	//{
	//	for (uint32_t i = 0; i < sizeof_array(c_speakersFull); ++i)
	//	{
	//		float* outputSamples = sfi->m_buffer[c_speakersFull[i].channel];
	//		T_ASSERT(alignUp(samples, 16) == samples);

	//		for (uint32_t j = 0; j < outBlock.samplesCount; j += 4)
	//			Vector4::zero().storeAligned(&outputSamples[j]);

	//		outBlock.samples[c_speakersFull[i].channel] = outputSamples;
	//	}
	//}

	//outBlock.maxChannel = c_speakersFullMaxChannel;
}

void SurroundFilter::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"speakerPosition", m_speakerPosition, AttributePoint());
	s >> Member< Scalar >(L"maxDistance", m_maxDistance);
}

}
