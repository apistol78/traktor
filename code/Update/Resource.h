#ifndef traktor_update_Resource_H
#define traktor_update_Resource_H

#include "Core/Misc/MD5.h"
#include "Update/Item.h"

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

/*! \brief Update resource.
 * \ingroup Update
 */
class T_DLLCLASS Resource : public Item
{
	T_RTTI_CLASS;

public:
	Resource();

	uint32_t getSize() const;

	const MD5& getMD5() const;

	const std::wstring& getUrl() const;

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_size;
	MD5 m_md5;
	std::wstring m_url;
};

	}
}

#endif	// traktor_update_Resource_H
