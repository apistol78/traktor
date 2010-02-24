#ifndef traktor_sound_PlayGrain_H
#define traktor_sound_PlayGrain_H

#include "Core/RefArray.h"
#include "Resource/Proxy.h"
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

class Sound;

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS PlayGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	PlayGrain();

	virtual bool bind(resource::IResourceManager* resourceManager);

	virtual Ref< ISoundBufferCursor > createCursor() const;

	virtual bool getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const;

	virtual bool serialize(ISerializer& s);

	const resource::Proxy< Sound >& getSound() const { return m_sound; }

	float getGain() const { return m_gain; }

private:
	mutable resource::Proxy< Sound > m_sound;
	float m_gain;
};

	}
}

#endif	// traktor_sound_PlayGrain_H
