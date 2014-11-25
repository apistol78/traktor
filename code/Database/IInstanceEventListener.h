#ifndef traktor_db_IInstanceEventListener_H
#define traktor_db_IInstanceEventListener_H

#include <string>

namespace traktor
{

class Guid;

	namespace db
	{

class Instance;

/*! \brief Instance event listener.
 * \ingroup Database
 */
class IInstanceEventListener
{
public:
	virtual ~IInstanceEventListener() {}

	virtual void instanceEventCreated(Instance* instance) = 0;

	virtual void instanceEventRemoved(Instance* instance) = 0;

	virtual void instanceEventGuidChanged(Instance* instance, const Guid& previousGuid) = 0;

	virtual void instanceEventRenamed(Instance* instance, const std::wstring& previousName) = 0;

	virtual void instanceEventCommitted(Instance* instance) = 0;
};

	}
}

#endif	// traktor_db_IInstanceEventListener_H
