#ifndef traktor_ui_custom_NumericPropertyItem_H
#define traktor_ui_custom_NumericPropertyItem_H

#include "Ui/Point.h"
#include "Ui/Custom/PropertyList/PropertyItem.h"

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

class Edit;

		namespace custom
		{

/*! \brief Numeric property item.
 * \ingroup UIC
 */
class T_DLLCLASS NumericPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	NumericPropertyItem(const std::wstring& text, double value, double limitMin, double limitMax, bool floatPoint = true, bool hex = false);

	void setValue(double value);

	double getValue() const;

	void setLimitMin(double limitMin);

	double getLimitMin() const;

	void setLimitMax(double limitMax);

	double getLimitMax() const;

	void setLimit(double limitMin, double limitMax);

protected:
	virtual void createInPlaceControls(Widget* parent);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void mouseButtonDown(MouseButtonDownEvent* event);

	virtual void mouseButtonUp(MouseButtonUpEvent* event);

	virtual void mouseMove(MouseMoveEvent* event);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

	virtual bool copy();

	virtual bool paste();

private:
	Ref< Edit > m_editor;
	double m_value;
	double m_limitMin;
	double m_limitMax;
	bool m_floatPoint;
	bool m_hex;
	bool m_mouseAdjust;
	Point m_mouseLastPosition;

	void eventEditFocus(FocusEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_NumericPropertyItem_H
