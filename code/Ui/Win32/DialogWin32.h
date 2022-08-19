#pragma once

#include "Ui/Itf/IDialog.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*!
 * \ingroup UIW32
 */
class DialogWin32 : public WidgetWin32Impl< IDialog >
{
public:
	explicit DialogWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) override;

	virtual void setIcon(ISystemBitmap* icon) override;

	virtual int showModal() override;

	virtual void endModal(int result) override;

	virtual void setMinSize(const Size& minSize) override;

	virtual void setVisible(bool visible) override;

	virtual Rect getRect() const override;

private:
	bool m_modal;
	Size m_minSize;
	int32_t m_centerStyle;
	bool m_keepCentered;
	DialogResult m_result;

	LRESULT eventInitDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventSizing(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventEndModal(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);
};

	}
}

