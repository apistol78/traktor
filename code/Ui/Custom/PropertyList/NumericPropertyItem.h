#ifndef traktor_ui_custom_NumericPropertyItem_H
#define traktor_ui_custom_NumericPropertyItem_H

#include "Ui/Custom/PropertyList/PropertyItem.h"
#include "Ui/Point.h"

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

class Edit;
class Event;

		namespace custom
		{

/*! \brief Numeric property item.
 * \ingroup UIC
 */
class T_DLLCLASS NumericPropertyItem : public PropertyItem
{
	T_RTTI_CLASS(NumericPropertyItem)

public:
	NumericPropertyItem(const std::wstring& text, double value, double min, double max, bool floatPoint = true, bool hex = false);

	void setValue(double value);

	double getValue() const;

	void setMin(double min);

	double getMin() const;

	void setMax(double max);

	double getMax() const;

	void setRange(double min, double max);

protected:
	virtual void createInPlaceControls(Widget* parent, bool visible);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void mouseButtonDown(MouseEvent* event);

	virtual void mouseButtonUp(MouseEvent* event);

	virtual void mouseMove(MouseEvent* event);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

private:
	Ref< Edit > m_editor;
	double m_value;
	double m_min;
	double m_max;
	bool m_floatPoint;
	bool m_hex;
	bool m_mouseAdjust;
	Point m_mouseLastPosition;

	void eventEditFocus(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_NumericPropertyItem_H
