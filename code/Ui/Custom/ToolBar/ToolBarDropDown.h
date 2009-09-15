#ifndef traktor_ui_custom_ToolBarDropDown_H
#define traktor_ui_custom_ToolBarDropDown_H

#include "Core/Heap/Ref.h"
#include "Ui/Custom/ToolBar/ToolBarItem.h"
#include "Ui/Command.h"
#include "Ui/Point.h"

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

/*! \brief Tool bar dropdown.
 * \ingroup UIC
 */
class T_DLLCLASS ToolBarDropDown : public ToolBarItem
{
	T_RTTI_CLASS(ToolBarDropDown)

public:
	ToolBarDropDown(const Command& command, int32_t width, const std::wstring& toolTip);

	int32_t add(const std::wstring& item);

	bool remove(int32_t index);

	void removeAll();

	int32_t count() const;

	std::wstring get(int32_t index) const;

	void select(int32_t index);

	int32_t getSelected() const;

	std::wstring getSelectedItem() const;

protected:
	virtual bool getToolTip(std::wstring& outToolTip) const;

	virtual Size getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const Point& at, Bitmap* images, int imageWidth, int imageHeight);

	virtual void mouseEnter(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void mouseLeave(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void buttonDown(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void buttonUp(ToolBar* toolBar, MouseEvent* mouseEvent);

private:
	Command m_command;
	int32_t m_width;
	std::wstring m_toolTip;
	std::vector< std::wstring > m_items;
	int32_t m_selected;
	bool m_hover;
	int32_t m_dropPosition;
	Point m_menuPosition;
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBarDropDown_H
