#ifndef traktor_sound_BankAsset_H
#define traktor_sound_BankAsset_H

#include "Core/RefArray.h"
#include "Editor/ITypedAsset.h"

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

class BankSound;
class IGrain;

class T_DLLCLASS BankAsset : public editor::ITypedAsset
{
	T_RTTI_CLASS;

public:
	void addGrain(IGrain* grain);

	const RefArray< IGrain >& getGrains() const;

	void addSound(BankSound* sound);

	const RefArray< BankSound >& getSounds() const;

	virtual const TypeInfo* getOutputType() const;

	virtual bool serialize(ISerializer& s);

private:
	RefArray< IGrain > m_grains;
	RefArray< BankSound > m_sounds;
};

	}
}

#endif	// traktor_sound_BankAsset_H
