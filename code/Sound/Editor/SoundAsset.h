#ifndef traktor_sound_SoundAsset_H
#define traktor_sound_SoundAsset_H

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS SoundAsset : public editor::Asset
{
	T_RTTI_CLASS(SoundAsset)

public:
	SoundAsset();

	virtual const Type* getOutputType() const;

	virtual bool serialize(Serializer& s);

private:
	friend class SoundPipeline;

	bool m_stream;
};

	}
}

#endif	// traktor_sound_SoundAsset_H
