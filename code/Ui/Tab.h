#ifndef traktor_ui_Tab_H
#define traktor_ui_Tab_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class TabPage;
class Bitmap;

/*! \brief Tab container.
 * \ingroup UI
 */
class T_DLLCLASS Tab : public Widget
{
	T_RTTI_CLASS(Tab)

public:
	enum
	{
		WsCloseButton = WsUser,
		WsLine = WsUser << 1,
		WsDefault = WsCloseButton | WsBorder
	};

	Tab();

	bool create(Widget* parent, int style = WsDefault);

	virtual Rect getInnerRect() const;

	int addImage(Bitmap* image, int imageCount);
	
	int addPage(TabPage* page);

	int getPageCount() const;

	TabPage* getPage(int index) const;

	TabPage* getPageAt(const Point& position) const;

	void removePage(TabPage* page);

	void removeAllPages();

	void setActivePage(TabPage* page);

	TabPage* getActivePage();

	TabPage* cycleActivePage(bool forward);
	
	void addSelChangeEventHandler(EventHandler* eventHandler);

	void addCloseEventHandler(EventHandler* eventHandler);

	virtual Size getMinimumSize() const;
	
	virtual Size getPreferedSize() const;

private:
	struct PageState
	{
		Ref< TabPage > page;
		int right;
		int depth;
	
		PageState(TabPage* page, int right = 0);
	
		bool operator == (const PageState& rh) const;
	};

	typedef std::vector< PageState > page_state_vector_t;

	Ref< Bitmap > m_image;
	uint32_t m_imageWidth;
	uint32_t m_imageHeight;
	Rect m_innerRect;
	Font m_fontBold;
	page_state_vector_t m_pages;
	Ref< TabPage > m_selectedPage;
	bool m_closeButton;
	bool m_drawBorder;
	bool m_drawLine;
	bool m_closeHighlight;

	PageState* findPageState(const TabPage* page);

	PageState* findPageState(int depth);

	void drawClose(Canvas& canvas, int x, int y);

	void eventMouseMove(Event* event);

	void eventButtonDown(Event* event);

	void eventSize(Event* event);

	void eventPaint(Event* event);
};

	}
}

#endif	// traktor_ui_Tab_H
