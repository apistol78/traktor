#include "Sound/Sound.h"
#include "Sound/Player/ISoundHandle.h"
#include "Sound/Player/ISoundPlayer.h"
#include "Spray/SoundTriggerInstance.h"
#include "Spray/Types.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

int32_t s_handleEnable;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SoundTriggerInstance", SoundTriggerInstance, ITriggerInstance)

SoundTriggerInstance::~SoundTriggerInstance()
{
	if (m_handle)
	{
		m_handle->stop();
		m_handle = 0;
	}
}

void SoundTriggerInstance::perform(Context& context, const Transform& transform, bool enable)
{
	if (!context.soundPlayer || !enable)
		return;
	
	if (m_handle)
	{
		m_handle->stop();
		m_handle = 0;
	}

	if (m_positional)
		m_handle = context.soundPlayer->play3d(m_sound, transform.translation(), 16);
	else
		m_handle = context.soundPlayer->play(m_sound, 16);
}

void SoundTriggerInstance::update(Context& context, const Transform& transform, bool enable)
{
	if (!m_handle)
		return;

	if (enable)
	{
		if (!m_handle->isPlaying())
		{
			if (m_repeat)
			{
				if (m_positional)
					m_handle = context.soundPlayer->play3d(m_sound, transform.translation(), 16);
				else
					m_handle = context.soundPlayer->play(m_sound, 16);
			}
			else
				m_handle = 0;
		}

		if (m_handle)
		{
			m_handle->setParameter(s_handleEnable, 1.0f);
			if (m_positional && m_follow)
				m_handle->setPosition(transform.translation());
		}
	}
	else
	{
		if (!m_infinite)
			m_handle->setParameter(s_handleEnable, 0.0f);
		else
			m_handle->stop();

		m_handle = 0;
	}
}

SoundTriggerInstance::SoundTriggerInstance(const resource::Proxy< sound::Sound >& sound, bool positional, bool follow, bool repeat, bool infinite)
:	m_sound(sound)
,	m_positional(positional)
,	m_follow(follow)
,	m_repeat(repeat)
,	m_infinite(infinite)
{
	s_handleEnable = sound::getParameterHandle(L"Enable");
}

	}
}
