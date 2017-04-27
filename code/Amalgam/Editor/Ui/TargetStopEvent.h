/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_TargetStopEvent_H
#define traktor_amalgam_TargetStopEvent_H

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

#endif	// traktor_amalgam_TargetStopEvent_H
