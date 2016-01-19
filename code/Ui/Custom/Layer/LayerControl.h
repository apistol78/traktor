#ifndef traktor_ui_custom_LayerControl_H
#define traktor_ui_custom_LayerControl_H

#include "Core/RefArray.h"
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

class IBitmap;
class ScrollBar;

		namespace custom
		{

class LayerItem;

/*! \brief Layer control.
 * \ingroup UIC
 */
class T_DLLCLASS LayerControl : public Widget
{
	T_RTTI_CLASS;

public:
	enum GetSequenceFlags
	{
		GfDefault = 0,
		GfDescendants = 1,
		GfSelectedOnly = 2
	};

	bool create(Widget* parent, int style = WsNone);

	void addLayerItem(LayerItem* layerItem);

	void removeLayerItem(LayerItem* layerItem);

	void removeAllLayerItems();

	RefArray< LayerItem >& getLayerItems();

	int getItems(RefArray< LayerItem >& outItems, int flags);

	Ref< LayerItem > getLayerItem(int index, bool includeChildren = true);

	virtual Size getPreferedSize() const;

private:
	Ref< ScrollBar > m_scrollBar;
	RefArray< LayerItem > m_layers;
	Ref< IBitmap > m_imageVisible;
	Ref< IBitmap > m_imageHidden;

	void updateScrollBar();

	void paintItem(Canvas& canvas, Rect& rcItem, LayerItem* item, int childLevel);

	void eventScroll(ScrollEvent* event);

	void eventSize(SizeEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_LayerControl_H
