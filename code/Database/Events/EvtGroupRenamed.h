#ifndef traktor_db_EvtGroupRenamed_H
#define traktor_db_EvtGroupRenamed_H

#include "Database/IEvent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class T_DLLCLASS EvtGroupRenamed : public IEvent
{
	T_RTTI_CLASS;

public:
	EvtGroupRenamed();

	EvtGroupRenamed(const std::wstring& name, const std::wstring& previousPath);

	const std::wstring& getName() const;

	const std::wstring& getPreviousPath() const;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_name;
	std::wstring m_previousPath;
};

	}
}

#endif	// traktor_db_EvtGroupRenamed_H
