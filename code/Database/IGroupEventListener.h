/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_IGroupEventListener_H
#define traktor_db_IGroupEventListener_H

#include <string>

namespace traktor
{
	namespace db
	{

class Group;

/*! \brief Group event listener.
 * \ingroup Database
 */
class IGroupEventListener
{
public:
	virtual ~IGroupEventListener() {}

	virtual void groupEventRenamed(Group* group, const std::wstring& previousName) = 0;
};

	}
}

#endif	// traktor_db_IGroupEventListener_H
