#ifndef traktor_ui_custom_VectorPropertyItem_H
#define traktor_ui_custom_VectorPropertyItem_H

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

/*! \brief Vector property item.
 * \ingroup UIC
 */
class T_DLLCLASS VectorPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	enum { MaxDimension = 4 };

	typedef float vector_t [MaxDimension];

	VectorPropertyItem(const std::wstring& text, const vector_t& value, int dimension = MaxDimension);

	void setValue(const vector_t& value);

	const vector_t& getValue() const;

protected:
	virtual void createInPlaceControls(Widget* parent);

	virtual void destroyInPlaceControls();

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects);

	virtual void mouseButtonDown(MouseButtonDownEvent* event);

	virtual void paintValue(Canvas& canvas, const Rect& rc);

private:
	Ref< Edit > m_editors[MaxDimension];
	vector_t m_value;
	int m_dimension;

	void eventEditFocus(FocusEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_VectorPropertyItem_H
