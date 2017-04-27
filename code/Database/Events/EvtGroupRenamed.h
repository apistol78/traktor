/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

/*! \brief Group renamed event.
 * \ingroup Database
 */
class T_DLLCLASS EvtGroupRenamed : public IEvent
{
	T_RTTI_CLASS;

public:
	EvtGroupRenamed();

	EvtGroupRenamed(const std::wstring& name, const std::wstring& previousPath);

	/*! \brief New group name.
	 *
	 * \return New group name.
	 */
	const std::wstring& getName() const;

	/*! \brief Path to group.
	 *
	 * This path also contain the current name
	 * of the group.
	 *
	 * \return Path to group.
	 */
	const std::wstring& getPreviousPath() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_name;
	std::wstring m_previousPath;
};

	}
}

#endif	// traktor_db_EvtGroupRenamed_H
