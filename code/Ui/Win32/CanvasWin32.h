/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_CanvasWin32_H
#define traktor_ui_CanvasWin32_H

#include "Ui/Itf/ICanvas.h"
#include "Ui/Win32/Window.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class CanvasWin32 : public ICanvas
{
public:
	/*!
	 * \param hWnd Target window.
	 * \param doubleBuffer Double buffered draw.
	 * \param hDC If non null, use context for drawing.
	 */
	virtual bool beginPaint(Window& hWnd, bool doubleBuffer, HDC hDC) = 0;

	virtual void endPaint(Window& hWnd) = 0;

	virtual Size getTextExtent(Window& hWnd, const std::wstring& text) const = 0;
};
	
	}
}

#endif	// traktor_ui_CanvasWin32_H
