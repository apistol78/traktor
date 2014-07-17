#ifndef traktor_ui_ScrollBar_H
#define traktor_ui_ScrollBar_H

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

/*! \brief Scroll bar.
 * \ingroup UI
 */
class T_DLLCLASS ScrollBar : public Widget
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsHorizontal = 0,
		WsVertical = WsUser
	};

	bool create(Widget* parent, int style = WsHorizontal);

	void setRange(int range);

	int getRange() const;

	void setPage(int page);

	int getPage() const;

	void setPosition(int position);

	int getPosition() const;
};

	}
}

#endif	// traktor_ui_ScrollBar_H
