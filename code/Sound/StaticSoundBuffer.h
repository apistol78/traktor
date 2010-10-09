#ifndef traktor_sound_StaticSoundBuffer_H
#define traktor_sound_StaticSoundBuffer_H

#include "Core/Misc/AutoPtr.h"
#include "Sound/ISoundBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{
	
class Instance;
	
	}
	
	namespace sound
	{

class StaticSoundResource;

/*! \brief Static sound buffer.
 * \ingroup Sound
 */
class T_DLLCLASS StaticSoundBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	virtual ~StaticSoundBuffer();

	bool create(const StaticSoundResource* resource, db::Instance* resourceInstance);

	void destroy();

	virtual Ref< ISoundBufferCursor > createCursor() const;

	virtual bool getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const;

private:
	Ref< const StaticSoundResource > m_resource;
	Ref< db::Instance > m_resourceInstance;
	mutable AutoArrayPtr< int16_t > m_buffer;
};

	}
}

#endif	// traktor_sound_StaticSoundBuffer_H
