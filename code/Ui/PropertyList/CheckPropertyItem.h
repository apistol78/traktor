/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_CheckPropertyItem_H
#define traktor_ui_CheckPropertyItem_H

#include "Ui/PropertyList/PropertyItem.h"

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

/*! \brief Check property item.
 * \ingroup UI
 */
class T_DLLCLASS CheckPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	CheckPropertyItem(const std::wstring& text, bool checked);

	void setChecked(bool checked);
	
	bool isChecked() const;

protected:
	virtual void mouseButtonDown(MouseButtonDownEvent* event) override;

	virtual void paintValue(Canvas& canvas, const Rect& rc) override;

private:
	bool m_checked;
};

	}
}

#endif	// traktor_ui_CheckPropertyItem_H
