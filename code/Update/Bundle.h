#ifndef traktor_update_Bundle_H
#define traktor_update_Bundle_H

#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UPDATE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace update
	{

class Item;

/*! \brief Update bundle.
 * \ingroup Update
 */
class T_DLLCLASS Bundle : public ISerializable
{
	T_RTTI_CLASS;

public:
	Bundle();

	int32_t getVersion() const;

	const std::wstring& getDescription() const;

	const RefArray< Item >& getItems() const;

	virtual bool serialize(ISerializer& s);

private:
	int32_t m_version;
	std::wstring m_description;
	RefArray< Item > m_items;
};

	}
}

#endif	// traktor_update_Bundle_H
