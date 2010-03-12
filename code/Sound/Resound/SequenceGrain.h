#ifndef traktor_sound_SequenceGrain_H
#define traktor_sound_SequenceGrain_H

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
class T_DLLCLASS SequenceGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	virtual bool bind(resource::IResourceManager* resourceManager);

	virtual Ref< ISoundBufferCursor > createCursor() const;

	virtual void updateCursor(ISoundBufferCursor* cursor) const;

	virtual const IGrain* getCurrentGrain(ISoundBufferCursor* cursor) const;

	virtual bool getBlock(const ISoundMixer* mixer, ISoundBufferCursor* cursor, SoundBlock& outBlock) const;

	virtual bool serialize(ISerializer& s);

	void addGrain(IGrain* grain) { m_grains.push_back(grain); }

	void removeGrain(IGrain* grain) { m_grains.remove(grain); }

	const RefArray< IGrain >& getGrains() const { return m_grains; }

private:
	RefArray< IGrain > m_grains;
};

	}
}

#endif	// traktor_sound_SequenceGrain_H
