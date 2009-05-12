#ifndef traktor_db_LocalFileLink_H
#define traktor_db_LocalFileLink_H

#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Virtual file link.
 * \ingroup Database
 */
class T_DLLCLASS LocalFileLink : public Serializable
{
	T_RTTI_CLASS(LocalFileLink)

public:
	virtual bool serialize(Serializer& s);

	inline const std::wstring& getPath() const { return m_path; }

private:
	std::wstring m_path;
};

	}
}

#endif	// traktor_db_LocalFileLink_H
