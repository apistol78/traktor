#ifndef traktor_flash_FlashSoundPlayer_H
#define traktor_flash_FlashSoundPlayer_H

#include "Core/Object.h"

namespace traktor
{
	namespace flash
	{

class FlashSound;
class ISoundRenderer;

class FlashSoundPlayer : public Object
{
	T_RTTI_CLASS;

public:
	FlashSoundPlayer(ISoundRenderer* soundRenderer);

	void play(const FlashSound& sound);

private:
	Ref< ISoundRenderer > m_soundRenderer;
};

	}
}

#endif	// traktor_flash_FlashSoundPlayer_H
