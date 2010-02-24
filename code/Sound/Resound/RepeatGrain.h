#ifndef traktor_sound_RepeatGrain_H
#define traktor_sound_RepeatGrain_H

#include "Core/RefArray.h"
#include "Core/Math/Random.h"
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
class T_DLLCLASS RepeatGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	RepeatGrain();

	virtual Ref< ISoundBufferCursor > createCursor(const BankBuffer* bank) const;

	virtual bool getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const;

	virtual bool serialize(ISerializer& s);

	uint32_t getCount() const { return m_count; }

	const Ref< IGrain >& getGrain() const { return m_grain; }

private:
	uint32_t m_count;
	Ref< IGrain > m_grain;
};

	}
}

#endif	// traktor_sound_RepeatGrain_H
