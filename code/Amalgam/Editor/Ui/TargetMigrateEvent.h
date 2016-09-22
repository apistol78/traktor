#ifndef traktor_amalgam_TargetMigrateEvent_H
#define traktor_amalgam_TargetMigrateEvent_H

#include "Ui/Event.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;

/*! \brief
 * \ingroup Amalgam
 */
class TargetMigrateEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	TargetMigrateEvent(ui::EventSubject* sender, TargetInstance* instance);

	TargetInstance* getInstance() const;

private:
	Ref< TargetInstance > m_instance;
};

	}
}

#endif	// traktor_amalgam_TargetMigrateEvent_H
