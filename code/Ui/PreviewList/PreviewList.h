#pragma once

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

	virtual void layoutCells(const Rect& rc) override final;

	void eventButtonDown(MouseButtonDownEvent* event);
};

	}
}

