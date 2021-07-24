#pragma once

#include "Ui/Event.h"
#include "Ui/Point.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! Mouse event.
 * \ingroup UI
 */
class T_DLLCLASS MouseWheelEvent : public Event
{
	T_RTTI_CLASS;

public:
	explicit MouseWheelEvent(EventSubject* sender, int32_t rotation, const ui::Point& position);

	int32_t getRotation() const;

	const ui::Point& getPosition() const;

private:
	int32_t m_rotation;
	ui::Point m_position;
};

	}
}

