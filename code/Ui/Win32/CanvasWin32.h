#pragma once

#include "Ui/Itf/ICanvas.h"
#include "Ui/Itf/IFontMetric.h"

namespace traktor
{
	namespace ui
	{

class Window;

/*! \brief
 * \ingroup UIW32
 */
class CanvasWin32
:	public ICanvas
,	public IFontMetric
{
public:
	/*!
	 * \param doubleBuffer Double buffered draw.
	 * \param hDC If non null, use context for drawing.
	 */
	virtual bool beginPaint(Window& hWnd, bool doubleBuffer, HDC hDC) = 0;

	virtual void endPaint(Window& hWnd) = 0;

	virtual void getAscentAndDescent(Window& hWnd, int32_t& outAscent, int32_t& outDescent) const = 0;

	virtual int32_t getAdvance(Window& hWnd, wchar_t ch, wchar_t next) const = 0;

	virtual int32_t getLineSpacing(Window& hWnd) const = 0;

	virtual Size getExtent(Window& hWnd, const std::wstring& text) const = 0;
};

	}
}

