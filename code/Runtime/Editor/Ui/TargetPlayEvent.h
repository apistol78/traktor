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
class TargetPlayEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	TargetPlayEvent(ui::EventSubject* sender, TargetInstance* instance);

	TargetInstance* getInstance() const;

private:
	Ref< TargetInstance > m_instance;
};

	}
}

