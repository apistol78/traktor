#ifndef traktor_ui_MouseEvent_H
#define traktor_ui_MouseEvent_H

#include "Ui/Event.h"
#include "Ui/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
	
/*! \brief Mouse event.
 * \ingroup UI
 */
class T_DLLCLASS MouseEvent : public Event
{
	T_RTTI_CLASS(MouseEvent)
	
public:
	enum Button
	{
		BtNone = 0,
		BtLeft = 1,
		BtMiddle = 2,
		BtRight = 4
	};

	MouseEvent(EventSubject* sender, Object* item, int button, const ui::Point& position, int wheelRotation = 0);
	
	int getButton() const;
	
	const ui::Point& getPosition() const;

	int getWheelRotation() const;
	
private:
	int m_button;
	ui::Point m_position;
	int m_wheelRotation;
};
	
	}
}

#endif	// traktor_ui_MouseEvent_H
