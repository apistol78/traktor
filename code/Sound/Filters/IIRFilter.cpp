/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Math/Const.h"
#include "Core/Math/Vector4.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Sound/Filters/IIRFilter.h"
#include "Sound/Filters/IIR/IIR.h"

namespace traktor::sound
{
	namespace
	{

struct IIRFilterInstance : public RefCountImpl< IAudioFilterInstance >
{
	AutoPtr< BiquadFilter > filter[SbcMaxChannelCount];
	BiquadsCascade bqc[SbcMaxChannelCount];
	int32_t lastSampleRate = 0;
	AlignedVector< double > tmp;

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

double hertzToRads(int32_t sampleRate, double hz)
{
	return hz * TWO_PI / sampleRate;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.IIRFilter", 0, IIRFilter, IAudioFilter)

Ref< IAudioFilterInstance > IIRFilter::createInstance() const
{
	return new IIRFilterInstance();
}

void IIRFilter::apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const
{
	IIRFilterInstance* lpfi = static_cast< IIRFilterInstance* >(instance);

	// Reconfigure filter if sample rate changes.
	if (outBlock.sampleRate != lpfi->lastSampleRate)
	{
		for (uint32_t i = 0; i < SbcMaxChannelCount; ++i)
		{
			switch (m_filterType)
			{
			case FilterType::Butterworth:
				lpfi->filter[i].reset(new ButterworthFilter());
				break;

			case FilterType::Chebyshev:
				lpfi->filter[i].reset(new ChebyshevFilter());
				break;

			case FilterType::Elliptic:
				lpfi->filter[i].reset(new EllipticFilter());
				break;
			}

			switch (m_filterMode)
			{
			case FilterMode::LowPass:
				lpfi->filter[i]->createLowPass(
					hertzToRads(outBlock.sampleRate, m_cutOffFrequency), m_passBand,
					hertzToRads(outBlock.sampleRate, m_stopFrequency), m_stopBand
				);
				break;

			case FilterMode::HighPass:
				lpfi->filter[i]->createHighPass(
					hertzToRads(outBlock.sampleRate, m_cutOffFrequency), m_passBand,
					hertzToRads(outBlock.sampleRate, m_stopFrequency), m_stopBand
				);
				break;
			}

			lpfi->bqc[i] = lpfi->filter[i]->biquadsCascade;
		}
		lpfi->lastSampleRate = outBlock.sampleRate;
	}

	// Apply filter to block.
	auto& tmp = lpfi->tmp;
	tmp.resize(outBlock.samplesCount);

	for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
	{
		float* samples = outBlock.samples[j];

		for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
			tmp[i] = (double)samples[i];

		lpfi->bqc[j].computeOutput(tmp.ptr(), outBlock.samplesCount);

		for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
			samples[i] = (float)tmp[i];
	}
}

void IIRFilter::serialize(ISerializer& s)
{
	const MemberEnum< FilterType >::Key c_FilterType_Keys[] =
	{
		{ L"Butterworth", FilterType::Butterworth },
		{ L"Chebyshev", FilterType::Chebyshev },
		{ L"Elliptic", FilterType::Elliptic },
		{ 0 }
	};

	const MemberEnum< FilterMode >::Key c_FilterMode_Keys[] =
	{
		{ L"LowPass", FilterMode::LowPass },
		{ L"HighPass", FilterMode::HighPass },
		{ 0 }
	};

	s >> MemberEnum< FilterType >(L"filterType", m_filterType, c_FilterType_Keys);
	s >> MemberEnum< FilterMode >(L"filterMode", m_filterMode, c_FilterMode_Keys);
	s >> Member< float >(L"cutOffFrequency", m_cutOffFrequency, AttributeUnit(UnitType::Hertz));
	s >> Member< float >(L"stopFrequency", m_stopFrequency, AttributeUnit(UnitType::Hertz));
	s >> Member< float >(L"passBand", m_passBand, AttributeUnit(UnitType::Decibel));
	s >> Member< float >(L"stopBand", m_stopBand, AttributeUnit(UnitType::Decibel));
}

}
