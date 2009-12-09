#ifndef traktor_sound_PlayGrain_H
#define traktor_sound_PlayGrain_H

#include "Core/RefArray.h"
#include "Sound/Resound/IGrain.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS PlayGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	PlayGrain();

	PlayGrain(int32_t index);

	virtual Ref< ISoundBufferCursor > createCursor(const BankBuffer* bank) const;

	virtual bool getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const;

	virtual bool serialize(ISerializer& s);

private:
	int32_t m_index;
};

	}
}

#endif	// traktor_sound_PlayGrain_H
