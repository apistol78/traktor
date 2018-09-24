/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_PreviewList_H
#define traktor_ui_PreviewList_H

#include "Core/Ref.h"
#include "Ui/Auto/AutoWidget.h"
#include "Ui/PreviewList/PreviewSelectionChangeEvent.h"

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

class PreviewItems;

class T_DLLCLASS PreviewList : public ui::AutoWidget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, uint32_t style);

	void setItems(PreviewItems* items);

	Ref< PreviewItems > getItems() const;
	
	PreviewItem* getSelectedItem() const;

private:
	Ref< PreviewItems > m_items;

	virtual void layoutCells(const Rect& rc) T_OVERRIDE T_FINAL;

	void eventButtonDown(MouseButtonDownEvent* event);
};

	}
}

#endif	// traktor_ui_PreviewList_H
