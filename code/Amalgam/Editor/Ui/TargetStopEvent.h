#pragma once

#include "Ui/Event.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;

/*! \brief
 * \ingroup Amalgam
 */
class TargetStopEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	TargetStopEvent(ui::EventSubject* sender, TargetInstance* instance, int32_t connectionIndex);

	TargetInstance* getInstance() const;

	int32_t getConnectionIndex() const;

private:
	Ref< TargetInstance > m_instance;
	int32_t m_connectionIndex;
};

	}
}

