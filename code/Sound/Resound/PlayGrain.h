#ifndef traktor_sound_PlayGrain_H
#define traktor_sound_PlayGrain_H

#include "Core/RefArray.h"
#include "Core/Math/Range.h"
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

class IFilter;
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

	virtual void updateCursor(ISoundBufferCursor* cursor) const;

	virtual const IGrain* getCurrentGrain(ISoundBufferCursor* cursor) const;

	virtual bool getBlock(const ISoundMixer* mixer, ISoundBufferCursor* cursor, SoundBlock& outBlock) const;

	virtual bool serialize(ISerializer& s);

	const resource::Proxy< Sound >& getSound() const { return m_sound; }

	const Range< float >& getGain() const { return m_gain; }

	const Range< float >& getPitch() const { return m_pitch; }

private:
	mutable resource::Proxy< Sound > m_sound;
	RefArray< IFilter > m_filters;
	Range< float > m_gain;
	Range< float > m_pitch;
	mutable Random m_random;
};

	}
}

#endif	// traktor_sound_PlayGrain_H
