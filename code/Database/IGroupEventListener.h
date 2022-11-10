#pragma once

#include <string>

namespace traktor::db
{

class Group;

/*! Group event listener.
 * \ingroup Database
 */
class IGroupEventListener
{
public:
	virtual ~IGroupEventListener() {}

	virtual void groupEventRenamed(Group* group, const std::wstring& previousName) = 0;
};

}
