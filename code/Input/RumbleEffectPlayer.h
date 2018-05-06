/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_RumbleEffectPlayer_H
#define traktor_input_RumbleEffectPlayer_H

#include <list>
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class RumbleEffect;
class IInputDevice;

/*! \brief Rumble effect player.
 * \ingroup Input
 */
class T_DLLCLASS RumbleEffectPlayer : public Object
{
	T_RTTI_CLASS;

public:
	RumbleEffectPlayer();

	virtual void play(RumbleEffect* effect, IInputDevice* targetDevice);

	virtual void stop(RumbleEffect* effect, IInputDevice* targetDevice);

	virtual void stopAll();

	virtual void update(float deltaTime);

private:
	struct PlayingEffect
	{
		Ref< RumbleEffect > effect;
		Ref< IInputDevice > targetDevice;
		float attachedTime;

		PlayingEffect(RumbleEffect* effect_, IInputDevice* targetDevice_, float attachedTime_);

		bool operator == (const PlayingEffect& other) const;
	};

	float m_totalTime;
	std::list< PlayingEffect > m_playingEffects;
};

	}
}

#endif	// traktor_input_RumbleEffectPlayer_H
