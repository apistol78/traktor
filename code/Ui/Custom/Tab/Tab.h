/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_Tab_H
#define traktor_ui_custom_Tab_H

#include "Ui/Widget.h"

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

class TabPage;

/*! \brief Tab control.
 * \ingroup UIC
 */
class T_DLLCLASS Tab : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent);

	virtual Rect getInnerRect() const T_OVERRIDE T_FINAL;
	
	virtual Size getPreferedSize() const T_OVERRIDE T_FINAL;
	
	int addPage(TabPage* page);

	int getPageCount() const;

	Ref< TabPage > getPage(int index);

	void removePage(TabPage* page);

	void removeAllPages();

	void setActivePage(TabPage* page);

	Ref< TabPage > getActivePage();

private:
	struct TabPagePair
	{
		Ref< TabPage > page;
		int right;
	
		TabPagePair(TabPage* page, int right = 0);
	
		bool operator == (const TabPagePair& rh) const;
	};
	typedef std::vector< TabPagePair > TabPageVector;

	Rect m_innerRect;
	Font m_fontBold;
	TabPageVector m_pages;
	Ref< TabPage > m_selectedPage;

	void drawClose(Canvas& canvas, int x, int y);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_Tab_H
