#ifndef traktor_ui_custom_ColorPropertyItem_H
#define traktor_ui_custom_ColorPropertyItem_H

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
		namespace custom
		{

/*! \brief Color property item.
 * \ingroup UIC
 */
class T_DLLCLASS ColorPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	ColorPropertyItem(const std::wstring& text, const Color4ub& value);

	void setValue(const Color4ub& value);

	const Color4ub& getValue() const;

protected:
	virtual void mouseButtonUp(MouseButtonUpEvent* event);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

private:
	Color4ub m_value;
	Rect m_rcColor;
};

		}
	}
}

#endif	// traktor_ui_custom_ColorPropertyItem_H
