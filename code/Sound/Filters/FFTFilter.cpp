#include <algorithm>
#include <cstring>
#include "Core/Math/Const.h"
#include "Core/Math/Hermite.h"
#include "Core/Math/Log2.h"
#include "Core/Math/MathUtils.h"
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Sound/Filters/FFTFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

uint32_t reverseBits(uint32_t value, uint32_t numBits)
{
	uint32_t i, rev;
	for(i = rev = 0; i < numBits; ++i)
	{
		rev = (rev << 1) | (value & 1);
		value >>= 1;
	}
	return rev;
}

void fft(
	uint32_t samplesCount,
	bool inverse,
	const float* realIn,
	const float* imagIn,
	float* realOut,
	float* imagOut
)
{
	uint32_t i, j, n, k;

	float angleNumerator = 2.0f * PI;
	if (inverse)
		angleNumerator = -angleNumerator;

	uint32_t numBits = log2(samplesCount);

	for (i = 0; i < samplesCount; ++i)
	{
		j = reverseBits(i, numBits);
		realOut[j] = realIn[i];
		imagOut[j] = imagIn != 0 ? imagIn[i] : 0.0f;
	}

	uint32_t blockEnd = 1;
	for (uint32_t blockSize = 2; blockSize <= samplesCount; blockSize <<= 1)
	{
		float deltaAngle = angleNumerator / float(blockSize);
		float sm2 = sinf(-2.0f * deltaAngle);
		float sm1 = sinf(-deltaAngle);
		float cm2 = cosf(-2.0f * deltaAngle);
		float cm1 = cosf(-deltaAngle);
		float w = 2.0f * cm1;

		for (i = 0; i < samplesCount; i += blockSize)
		{
			float ar[3], ai[3];

			ar[2] = cm2;
			ar[1] = cm1;

			ai[2] = sm2;
			ai[1] = sm1;

			for (j = i, n = 0; n < blockEnd; ++j, ++n)
			{
				ar[0] = w * ar[1] - ar[2];
				ar[2] = ar[1];
				ar[1] = ar[0];

				ai[0] = w * ai[1] - ai[2];
				ai[2] = ai[1];
				ai[1] = ai[0];

				k = j + blockEnd;

				float tr = ar[0] * realOut[k] - ai[0] * imagOut[k];
				float ti = ar[0] * imagOut[k] + ai[0] * realOut[k];

				realOut[k] = realOut[j] - tr;
				imagOut[k] = imagOut[j] - ti;

				realOut[j] += tr;
				imagOut[j] += ti;
			}
		}

		blockEnd = blockSize;
	}

	if (inverse)
	{
		float invDenom = 1.0f / float(samplesCount);
		for (i = 0; i < samplesCount; ++i)
		{
			realOut[i] *= invDenom;
			imagOut[i] *= invDenom;
		}
	}
}

struct PairAccessor
{
	static inline float time(const std::pair< float, float >* keys, size_t nkeys, const std::pair< float, float >& key)
	{
		return key.first;
	}

	static inline float value(const std::pair< float, float >& key)
	{
		return key.second;
	}

	static inline float combine(
		float t,
		float v0, float w0,
		float v1, float w1,
		float v2, float w2,
		float v3, float w3
	)
	{
		return v0 * w0 + v1 * w1 + v2 * w2 + v3 * w3;
	}
};

struct FFTFilterInstance : public RefCountImpl< IFilterInstance >
{
	float m_history[2][FFTFilter::N];

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

			}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.FFTFilter", 0, FFTFilter, IFilter)

FFTFilter::FFTFilter(uint32_t sampleRate)
:	m_sampleRate(sampleRate)
{
	for (uint32_t i = 0; i < N; ++i)
		m_filter[i] = 0.0f;
}

void FFTFilter::setFilter(const std::vector< std::pair< float, float > >& filter, uint32_t windowWidth)
{
	// Calculate filter in frequency domain.
	float fr[N], fi[N];
	for (uint32_t i = 0; i < N / 2; ++i)
	{
		float Hz = float(i * m_sampleRate) / (N / 2 - 1);
		float Fr = Hermite< std::pair< float, float >, float, PairAccessor >(&filter[0], filter.size()).evaluate(Hz);

		fr[i] = std::max(Fr, 0.0f);
		fi[i] = 0.0f;

		fr[N - i - 1] =  fr[i];
		fi[N - i - 1] = -fi[i];
	}

	// Transform filter into time domain.
	float tr[N], ti[N];
	fft(N, true, fr, fi, tr, ti);

	// Calculate Hanning window.
	if (windowWidth > 0)
	{
		float window[N];
		for (uint32_t i = 0; i < N; ++i)
		{
			if (i <= windowWidth / 2)
				window[i] = 0.5f + 0.5f * cosf(i * PI * 2.0f / (windowWidth + 1));
			else if (i >= N - windowWidth / 2)
				window[i] = 0.5f + 0.5f * cosf((N - 1 - i) * PI * 2.0f / (windowWidth + 1));
			else
				window[i] = 0.0f;
		}

		// Apply window in time domain.
		for (uint32_t i = 0; i < N; ++i)
		{
			tr[i] *= window[i];
			ti[i] = 0.0f;
		}
	}

	// Transform filter back to frequency domain.
	fft(N, false, tr, ti, fr, fi);

	// Calculate filter magnitudes.
	for (uint32_t i = 0; i < N; ++i)
		m_filter[i] = sqrtf(fr[i] * fr[i] + fi[i] * fi[i]);
}

Ref< IFilterInstance > FFTFilter::createInstance() const
{
	Ref< FFTFilterInstance > instance = new FFTFilterInstance();
	for (uint32_t i = 0; i < N; ++i)
	{
		instance->m_history[0][i] = 0.0f;
		instance->m_history[1][i] = 0.0f;
	}
	return instance;
}

void FFTFilter::apply(IFilterInstance* instance, SoundBlock& outBlock) const
{
	FFTFilterInstance* ffti = static_cast< FFTFilterInstance* >(instance);

	float fr[N], fi[N], dummy[N];

	T_ASSERT (outBlock.samplesCount % N == 0);

	const int n1 = N / 4;
	const int n2 = (N * 2) / 4;
	const int n3 = (N * 3) / 4;

	for (uint32_t i = 0; i < outBlock.maxChannel; ++i)
	{
		float* history = ffti->m_history[i];
		float* samples = outBlock.samples[i];
		
		for (uint32_t j = 0; j < outBlock.samplesCount; j += n1)
		{
			float buf[N];	// HHHS
			for (int k = 0; k < n3; ++k)
				buf[k] = history[k];
			for (int k = 0; k < n1; ++k)
				buf[k + n3] = samples[j + k];

			fft(N, false, buf, 0, fr, fi);

			for (uint32_t k = 0; k < N; ++k)
			{
				fr[k] *= m_filter[k];
				fi[k] *= m_filter[k];
			}

			fft(N, true, fr, fi, buf, dummy);

			std::memmove(history, &history[n1], n2 * sizeof(float));
			std::memcpy(&history[n2], &samples[j], n1 * sizeof(float));
			std::memcpy(&samples[j], &buf[n2], n1 * sizeof(float));
		}
	}
}

void FFTFilter::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"sampleRate", m_sampleRate);
	s >> MemberStaticArray< float, N >(L"filter", m_filter);
}

	}
}
