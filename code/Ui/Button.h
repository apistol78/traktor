#ifndef traktor_ui_Button_H
#define traktor_ui_Button_H

#include "Ui/Widget.h"

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

/*! \brief Button
 * \ingroup UI
 */
class T_DLLCLASS Button : public Widget
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsDefaultButton	= WsUser,
		WsToggle = (WsUser << 1)
	};

	bool create(Widget* parent, const std::wstring& text, int style = WsNone);

	void setState(bool state);

	bool getState() const;
};

	}
}

#endif	// traktor_ui_Button_H
