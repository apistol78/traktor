#ifndef traktor_db_DbmOpen_H
#define traktor_db_DbmOpen_H

#include "Database/Remote/IMessage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_REMOTE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

/*! \brief Open database.
 * \ingroup Database
 */
class T_DLLCLASS DbmOpen : public IMessage
{
	T_RTTI_CLASS;

public:
	DbmOpen(const std::wstring& name = L"");

	const std::wstring& getName() const { return m_name; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_name;
};

	}
}

#endif	// traktor_db_DbmOpen_H
