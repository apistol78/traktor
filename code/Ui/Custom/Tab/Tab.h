#ifndef traktor_ui_custom_Tab_H
#define traktor_ui_custom_Tab_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(Tab)

public:
	bool create(Widget* parent);

	virtual Rect getInnerRect() const;
	
	virtual Size getPreferedSize() const;
	
	int addPage(TabPage* page);

	int getPageCount() const;

	TabPage* getPage(int index);

	void removePage(TabPage* page);

	void removeAllPages();

	void setActivePage(TabPage* page);

	TabPage* getActivePage();
	
	void addSelChangeEventHandler(EventHandler* eventHandler);

	void addCloseEventHandler(EventHandler* eventHandler);

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

	void eventButtonDown(Event* event);

	void eventSize(Event* event);

	void eventPaint(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_Tab_H
