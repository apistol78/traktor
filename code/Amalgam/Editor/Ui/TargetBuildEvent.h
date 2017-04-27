/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_TargetBuildEvent_H
#define traktor_amalgam_TargetBuildEvent_H

#include "Ui/Event.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;

/*! \brief
 * \ingroup Amalgam
 */
class TargetBuildEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	TargetBuildEvent(ui::EventSubject* sender, TargetInstance* instance);

	TargetInstance* getInstance() const;

private:
	Ref< TargetInstance > m_instance;
};

	}
}

#endif	// traktor_amalgam_TargetBuildEvent_H
