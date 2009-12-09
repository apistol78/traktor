#ifndef traktor_sound_IGrain_H
#define traktor_sound_IGrain_H

#include "Core/Serialization/ISerializable.h"
#include "Sound/Types.h"

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

class BankBuffer;
class ISoundBufferCursor;

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS IGrain : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< ISoundBufferCursor > createCursor(const BankBuffer* bank) const = 0;

	virtual bool getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const = 0;
};

	}
}

#endif	// traktor_sound_IGrain_H
