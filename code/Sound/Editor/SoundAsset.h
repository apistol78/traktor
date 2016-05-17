#ifndef traktor_sound_SoundAsset_H
#define traktor_sound_SoundAsset_H

#include "Core/Guid.h"
#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS SoundAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	SoundAsset();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	float getPresence() const { return m_presence; }

	float getPresenceRate() const { return m_presenceRate; }

	float getVolume() const { return m_volume; }

private:
	friend class SoundPipeline;

	Guid m_category;
	bool m_stream;
	bool m_preload;
	float m_presence;
	float m_presenceRate;
	float m_volume;
};

	}
}

#endif	// traktor_sound_SoundAsset_H
