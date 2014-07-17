#ifndef traktor_ui_custom_ColorEvent_H
#define traktor_ui_custom_ColorEvent_H

#include "Ui/Event.h"
#include "Core/Math/Color4ub.h"

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

/*! \brief Color change event.
 * \ingroup UIC
 */
class T_DLLCLASS ColorEvent : public Event
{
	T_RTTI_CLASS;

public:
	ColorEvent(EventSubject* sender, const Color4ub& color);

	const Color4ub& getColor() const;

private:
	Color4ub m_color;
};

		}
	}
}

#endif	// traktor_ui_custom_ColorEvent_H
