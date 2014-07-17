#ifndef traktor_ui_custom_SequenceButtonClickEvent_H
#define traktor_ui_custom_SequenceButtonClickEvent_H

#include "Ui/Events/ButtonClickEvent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class Sequence;

/*! \brief
 * \ingroup UIC
 */
class T_DLLCLASS SequenceButtonClickEvent : public ButtonClickEvent
{
	T_RTTI_CLASS;

public:
	SequenceButtonClickEvent(EventSubject* sender, Sequence* sequence, const Command& command);

	Sequence* getSequence() const;

private:
	Ref< Sequence > m_sequence;
};

		}
	}
}

#endif	// traktor_ui_custom_SequenceButtonClickEvent_H
