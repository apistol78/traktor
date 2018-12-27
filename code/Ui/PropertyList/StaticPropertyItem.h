/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_StaticPropertyItem_H
#define traktor_ui_StaticPropertyItem_H

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

/*! \brief Static property item.
 * \ingroup UI
 */
class T_DLLCLASS StaticPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	StaticPropertyItem(const std::wstring& text, const std::wstring& value);

	void setValue(const std::wstring& text);

	const std::wstring& getValue() const;

protected:
	virtual void paintValue(Canvas& canvas, const Rect& rc) override;

private:
	std::wstring m_value;
};

	}
}

#endif	// traktor_ui_StaticPropertyItem_H
