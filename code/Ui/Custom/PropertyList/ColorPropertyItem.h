#ifndef traktor_ui_custom_ColorPropertyItem_H
#define traktor_ui_custom_ColorPropertyItem_H

#include "Ui/Custom/PropertyList/PropertyItem.h"

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

class Event;

		namespace custom
		{

/*! \brief Color property item.
 * \ingroup UIC
 */
class T_DLLCLASS ColorPropertyItem : public PropertyItem
{
	T_RTTI_CLASS(ColorPropertyItem)

public:
	ColorPropertyItem(const std::wstring& text, const Color& value);

	void setValue(const Color& value);

	const Color& getValue() const;

protected:
	virtual void mouseButtonUp(MouseEvent* event);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

private:
	Color m_value;
	Rect m_rcColor;
};

		}
	}
}

#endif	// traktor_ui_custom_ColorPropertyItem_H
