#ifndef traktor_amalgam_TargetCaptureEvent_H
#define traktor_amalgam_TargetCaptureEvent_H

#include "Ui/Event.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;

class TargetCaptureEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	TargetCaptureEvent(ui::EventSubject* sender, TargetInstance* instance, int32_t connectionIndex);

	TargetInstance* getInstance() const;

	int32_t getConnectionIndex() const;

private:
	Ref< TargetInstance > m_instance;
	int32_t m_connectionIndex;
};

	}
}

#endif	// traktor_amalgam_TargetCaptureEvent_H
