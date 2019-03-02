#pragma once

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

