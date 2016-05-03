#ifndef traktor_ui_custom_AnglesPropertyItem_H
#define traktor_ui_custom_AnglesPropertyItem_H

#include "Core/Math/Vector4.h"
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

/*! \brief Angles property item.
 * \ingroup UIC
 */
class T_DLLCLASS AnglesPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	AnglesPropertyItem(const std::wstring& text, const Vector4& value);

	void setValue(const Vector4& value);

	const Vector4& getValue() const;

protected:
	virtual void createInPlaceControls(Widget* parent) T_OVERRIDE;

	virtual void destroyInPlaceControls() T_OVERRIDE;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) T_OVERRIDE;

	virtual void mouseButtonDown(MouseButtonDownEvent* event) T_OVERRIDE;

	virtual void paintValue(Canvas& canvas, const Rect& rc) T_OVERRIDE;

private:
	Ref< Edit > m_editors[3];
	Vector4 m_value;

	void eventEditFocus(FocusEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_AnglesPropertyItem_H
