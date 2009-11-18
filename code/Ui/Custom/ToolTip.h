#ifndef traktor_ui_custom_ToolTip_H
#define traktor_ui_custom_ToolTip_H

#include "Ui/ToolForm.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Tool tip control.
 * \ingroup UIC
 */
class T_DLLCLASS ToolTip : public ToolForm
{
	T_RTTI_CLASS;

public:
	enum Events
	{
		EiShowTip = ui::EiUser
	};

	ToolTip();

	bool create(Widget* parent);

	void show(const Point& at, const std::wstring& text);

	void addShowEventHandler(EventHandler* eventHandler);

private:
	bool m_tracking;
	uint32_t m_counter;

	void eventTimer(Event* event);

	void eventPaint(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ToolTip_H
