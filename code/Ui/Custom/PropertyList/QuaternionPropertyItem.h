#ifndef traktor_ui_custom_QuaternionPropertyItem_H
#define traktor_ui_custom_QuaternionPropertyItem_H

#include "Core/Math/Quaternion.h"
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
class Event;

		namespace custom
		{

/*! \brief Quaternion property item.
 * \ingroup UIC
 */
class T_DLLCLASS QuaternionPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	QuaternionPropertyItem(const std::wstring& text, const Quaternion& value);

	void setValue(const Quaternion& value);

	const Quaternion& getValue() const;

protected:
	virtual void createInPlaceControls(Widget* parent);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void mouseButtonDown(MouseEvent* event);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

private:
	Ref< Edit > m_editors[3];
	Quaternion m_value;

	void eventEditFocus(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_QuaternionPropertyItem_H
