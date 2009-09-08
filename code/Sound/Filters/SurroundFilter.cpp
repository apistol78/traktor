#include "Sound/Filters/SurroundFilter.h"
#include "Sound/Filters/SurroundEnvironment.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace sound
	{

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

void SurroundFilter::apply(SoundBlock& outBlock)
{
	const struct Listener
	{
		float angle;
		float angleOffsetMax;
		int channel;
	}
	c_listenerCones[] =
	{
		{ 0.0f, 45.0f, SbcSideRight },
		{ 45.0f, 45.0f, SbcRight },
		{ 90.0f, 45.0f, SbcCenter },
		{ 135.0f, 45.0f, SbcLeft },
		{ 180.0f, 45.0f, SbcSideLeft },
		{ 235.0f, 65.0f, SbcRearLeft },
		{ 305.0f, 65.0f, SbcRearRight }
	};

	// Collapse all channels; should be mono sound sources.
	for (uint32_t i = 0; i < outBlock.samplesCount; ++i)
	{
		float sample = 0.0f;
		for (uint32_t j = 0; j < outBlock.maxChannel; ++j)
			sample += outBlock.samples[j][i];
		for (uint32_t j = 0; j < sizeof_array(c_listenerCones); ++j)
			m_buffer[c_listenerCones[j].channel][i] = sample;
		m_buffer[SbcLfe][i] = 0.0f;
	}
	outBlock.maxChannel = SbcMaxChannelCount;
	for (uint32_t j = 0; j < SbcMaxChannelCount; ++j)
		outBlock.samples[j] = m_buffer[j];

	// Get speaker position in listener space.
	Matrix44 listenerInvTransform = m_environment->getListenerTransform().inverseOrtho();
	Vector4 speakerPosition = listenerInvTransform * m_speakerPosition;

	// Speaker direction in XZ plane.
	Vector4 speakerDirection = speakerPosition * Vector4(1.0f, 0.0f, 1.0f, 0.0f);
	Scalar speakerDistance = speakerDirection.length();

	// Outside listener circle.
	if (speakerDistance > Scalar(1.0f))
	{
		speakerDirection /= speakerDistance;
		float angle = rad2deg(atan2f(-speakerDirection.z(), -speakerDirection.x()) + PI);
		for (uint32_t i = 0; i < sizeof_array(c_listenerCones); ++i)
		{
			float angleOffset = abs(angle - c_listenerCones[i].angle);
			while (angleOffset > 180.0f)
				angleOffset -= 360.0f;

			float attenuate = 1.0f - abs(angleOffset) / c_listenerCones[i].angleOffsetMax;
			attenuate = min(attenuate, 1.0f);
			attenuate = max(attenuate, 0.0f);

			for (uint32_t j = 0; j < outBlock.samplesCount; ++j)
			{
				outBlock.samples[c_listenerCones[i].channel][j] *= attenuate;
				outBlock.samples[SbcLfe][j] += outBlock.samples[c_listenerCones[i].channel][j];
			}
		}
	}
}

	}
}
