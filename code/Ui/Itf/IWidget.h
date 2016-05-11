#ifndef traktor_ui_IWidget_H
#define traktor_ui_IWidget_H

#include <string>
#include "Core/Config.h"
#include "Core/Platform.h"
#include "Ui/Enums.h"
#include "Ui/Rect.h"
#include "Ui/Font.h"

namespace traktor
{
	namespace ui
	{

struct IWidgetRect
{
	class IWidget* widget;
	Rect rect;
};

/*! \brief Widget interface.
 * \ingroup UI
 */
class IWidget
{
public:
	virtual ~IWidget() {}

	virtual void destroy() = 0;

	virtual void setParent(IWidget* parent) = 0;

	virtual void setText(const std::wstring& text) = 0;

	virtual std::wstring getText() const = 0;

	virtual void setToolTipText(const std::wstring& text) = 0;

	virtual void setForeground() = 0;

	virtual bool isForeground() const = 0;

	virtual void setVisible(bool visible) = 0;

	virtual bool isVisible(bool includingParents) const = 0;

	virtual void setActive() = 0;

	virtual void setEnable(bool enable) = 0;

	virtual bool isEnable() const = 0;

	virtual bool hasFocus() const = 0;

	virtual bool containFocus() const = 0;

	virtual void setFocus() = 0;

	virtual bool hasCapture() const = 0;

	virtual void setCapture() = 0;

	virtual void releaseCapture() = 0;

	virtual void startTimer(int interval, int id) = 0;

	virtual void stopTimer(int id) = 0;

	virtual void setOutline(const Point* p, int np) = 0;

	virtual void setRect(const Rect& rect) = 0;

	virtual Rect getRect() const = 0;

	virtual Rect getInnerRect() const = 0;

	virtual Rect getNormalRect() const = 0;

	virtual Size getTextExtent(const std::wstring& text) const = 0;

	virtual void setFont(const Font& font) = 0;

	virtual Font getFont() const = 0;

	virtual void setCursor(Cursor cursor) = 0;
	
	virtual Point getMousePosition(bool relative) const = 0;

	virtual Point screenToClient(const Point& pt) const = 0;

	virtual Point clientToScreen(const Point& pt) const = 0;

	virtual bool hitTest(const Point& pt) const = 0;

	virtual void setChildRects(const std::vector< IWidgetRect >& childRects) = 0;

	virtual Size getMinimumSize() const = 0;

	virtual Size getPreferedSize() const = 0;

	virtual Size getMaximumSize() const = 0;

	virtual void update(const Rect* rc, bool immediate) = 0;

	virtual void* getInternalHandle() = 0;

	virtual SystemWindow getSystemWindow() = 0;
};

	}
}

#endif	// traktor_ui_IWidget_H
