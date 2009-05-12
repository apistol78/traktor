#ifndef traktor_input_RumbleEffectPlayer_H
#define traktor_input_RumbleEffectPlayer_H

#include <list>
#include "Core/Heap/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class RumbleEffect;
class InputDevice;

class T_DLLCLASS RumbleEffectPlayer : public Object
{
	T_RTTI_CLASS(RumbleEffectPlayer)

public:
	RumbleEffectPlayer();

	void play(RumbleEffect* effect, InputDevice* targetDevice);

	void stop(RumbleEffect* effect, InputDevice* targetDevice);

	void stopAll();

	void update(float deltaTime);

private:
	struct PlayingEffect
	{
		Ref< RumbleEffect > effect;
		Ref< InputDevice > targetDevice;
		float attachedTime;

		PlayingEffect(RumbleEffect* effect_, InputDevice* targetDevice_, float attachedTime_);

		bool operator == (const PlayingEffect& other) const;
	};

	float m_totalTime;
	std::list< PlayingEffect > m_playingEffects;
};

	}
}

#endif	// traktor_input_RumbleEffectPlayer_H
