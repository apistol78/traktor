#pragma once

#include "Ui/Event.h"

namespace traktor
{
	namespace runtime
	{

class TargetInstance;

/*! \brief
 * \ingroup Runtime
 */
class TargetBrowseEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	TargetBrowseEvent(ui::EventSubject* sender, TargetInstance* instance);

	TargetInstance* getInstance() const;

private:
	Ref< TargetInstance > m_instance;
};

	}
}

