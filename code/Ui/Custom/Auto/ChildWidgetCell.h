#ifndef traktor_ui_custom_ChildWidgetCell_H
#define traktor_ui_custom_ChildWidgetCell_H

#include "Ui/Widget.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

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

/*! \brief Auto widget cell.
 * \ingroup UIC
 */
class T_DLLCLASS ChildWidgetCell : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	ChildWidgetCell(Widget* child);

	virtual ~ChildWidgetCell();

	virtual void placeCells(AutoWidget* widget, const Rect& rect) T_OVERRIDE;

	Widget* getChild() const;

private:
	Ref< Widget > m_child;
};

		}
	}
}

#endif	// traktor_ui_custom_ChildWidgetCell_H

