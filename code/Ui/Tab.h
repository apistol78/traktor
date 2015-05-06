#ifndef traktor_ui_Tab_H
#define traktor_ui_Tab_H

#include "Ui/Widget.h"

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

class TabPage;
class Bitmap;

/*! \brief Tab container.
 * \ingroup UI
 */
class T_DLLCLASS Tab : public Widget
{
	T_RTTI_CLASS;

public:
	enum
	{
		WsCloseButton = WsUser,
		WsLine = WsUser << 1,
		WsBottom = WsUser << 2,
		WsDefault = WsCloseButton | WsBorder
	};

	Tab();

	bool create(Widget* parent, int32_t style = WsDefault);

	virtual Rect getInnerRect() const;

	int32_t addImage(Bitmap* image, int32_t imageCount);
	
	int32_t addPage(TabPage* page);

	int32_t getPageCount() const;

	Ref< TabPage > getPage(int32_t index) const;

	Ref< TabPage > getPageAt(const Point& position) const;

	void removePage(TabPage* page);

	void removeAllPages();

	void setActivePage(TabPage* page);

	Ref< TabPage > getActivePage();

	Ref< TabPage > cycleActivePage(bool forward);
	
	void addSelChangeEventHandler(EventHandler* eventHandler);

	void addCloseEventHandler(EventHandler* eventHandler);

	virtual Size getMinimumSize() const;
	
	virtual Size getPreferedSize() const;

private:
	struct PageState
	{
		Ref< TabPage > page;
		int32_t right;
		int32_t depth;
	
		PageState(TabPage* page, int32_t right = 0);
	
		bool operator == (const PageState& rh) const;
	};

	typedef std::vector< PageState > page_state_vector_t;

	Ref< Bitmap > m_bitmapClose;
	Ref< Bitmap > m_bitmapImages;
	uint32_t m_imageWidth;
	uint32_t m_imageHeight;
	Rect m_innerRect;
	Font m_fontBold;
	page_state_vector_t m_pages;
	Ref< TabPage > m_selectedPage;
	Ref< TabPage > m_hoverPage;
	bool m_closeButton;
	bool m_drawBorder;
	bool m_drawLine;
	bool m_bottom;

	PageState* findPageState(const TabPage* page);

	PageState* findPageState(int32_t depth);

	void eventMouseMove(MouseMoveEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);

#if defined(_DEBUG)
	void checkPageStates();
#endif
};

	}
}

#endif	// traktor_ui_Tab_H
