/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	VectorPropertyItem(const std::wstring& text, const vector_t& value, int32_t dimension = MaxDimension);

	void setValue(const vector_t& value);

	const vector_t& getValue() const;

protected:
	virtual void createInPlaceControls(Widget* parent) T_OVERRIDE;

	virtual void destroyInPlaceControls() T_OVERRIDE;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) T_OVERRIDE;

	virtual void mouseButtonDown(MouseButtonDownEvent* event) T_OVERRIDE;

	virtual void paintValue(Canvas& canvas, const Rect& rc) T_OVERRIDE;

	virtual bool copy() T_OVERRIDE;

	virtual bool paste() T_OVERRIDE;

private:
	Ref< Edit > m_editors[MaxDimension];
	vector_t m_value;
	int32_t m_dimension;

	bool isEditing() const;

	void eventEditFocus(FocusEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_VectorPropertyItem_H
