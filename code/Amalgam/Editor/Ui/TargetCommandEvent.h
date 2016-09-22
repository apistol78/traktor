#ifndef traktor_amalgam_TargetCommandEvent_H
#define traktor_amalgam_TargetCommandEvent_H

#include "Ui/Event.h"

namespace traktor
{
	namespace amalgam
	{

class TargetInstance;

/*! \brief
 * \ingroup Amalgam
 */
class TargetCommandEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	TargetCommandEvent(ui::EventSubject* sender, TargetInstance* instance, int32_t connectionIndex, const std::wstring& command);

	TargetInstance* getInstance() const;

	int32_t getConnectionIndex() const;

	const std::wstring& getCommand() const;

private:
	Ref< TargetInstance > m_instance;
	int32_t m_connectionIndex;
	std::wstring m_command;
};

	}
}

#endif	// traktor_amalgam_TargetCommandEvent_H
