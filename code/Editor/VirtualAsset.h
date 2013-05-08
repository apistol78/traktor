#ifndef traktor_editor_VirtualAsset_H
#define traktor_editor_VirtualAsset_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

/*! \brief Virtual asset.
 * \ingroup Editor
 *
 * Virtual assets are used to masquerade source assets
 * into having other name and/or guid in the output database.
 *
 * Useful for making different permutations of data for
 * different SKUs etc.
 */
class T_DLLCLASS VirtualAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	const Guid& getSourceInstance() const;

	const Guid& getPlaceholderInstance() const;

	virtual void serialize(ISerializer& s);

private:
	Guid m_sourceInstance;
	Guid m_placeholderInstance;
};

	}
}

#endif	// traktor_editor_VirtualAsset_H
