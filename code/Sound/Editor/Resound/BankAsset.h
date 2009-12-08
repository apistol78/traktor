#ifndef traktor_sound_BankAsset_H
#define traktor_sound_BankAsset_H

#include <vector>
#include "Editor/ITypedAsset.h"
#include "Resource/Proxy.h"

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

class Sound;

class T_DLLCLASS BankAsset : public editor::ITypedAsset
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfo* getOutputType() const;

	virtual bool serialize(ISerializer& s);

	const std::vector< resource::Proxy< Sound > >& getSounds() const { return m_sounds; }

private:
	std::vector< resource::Proxy< Sound > > m_sounds;
};

	}
}

#endif	// traktor_sound_BankAsset_H
