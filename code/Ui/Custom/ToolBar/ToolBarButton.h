#ifndef traktor_ui_custom_ToolBarButton_H
#define traktor_ui_custom_ToolBarButton_H

#include "Ui/Command.h"
#include "Ui/Custom/ToolBar/ToolBarItem.h"

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

/*! \brief Tool bar button.
 * \ingroup UIC
 */
class T_DLLCLASS ToolBarButton : public ToolBarItem
{
	T_RTTI_CLASS;

public:
	enum ButtonStyles
	{
		BsIcon = 1 << 0,
		BsText = 1 << 1,
		BsToggle = 1 << 2,
		BsToggled = BsToggle | (1 << 3),
		BsDefault = BsIcon,
		BsDefaultToggle = BsDefault | BsToggle,
		BsDefaultToggled = BsDefault | BsToggled
	};

	ToolBarButton(const std::wstring& text, uint32_t imageIndex, const Command& command, int32_t style = BsIcon);

	ToolBarButton(const std::wstring& text, const Command& command, int32_t style = BsText);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setImage(uint32_t imageIndex);

	uint32_t getImage() const;

	void setToggled(bool toggled);

	bool isToggled() const;

protected:
	virtual bool getToolTip(std::wstring& outToolTip) const;

	virtual Size getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const Point& at, Bitmap* images, int imageWidth, int imageHeight);

	virtual bool mouseEnter(ToolBar* toolBar, MouseMoveEvent* mouseEvent);

	virtual void mouseLeave(ToolBar* toolBar, MouseMoveEvent* mouseEvent);

	virtual void buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent);

	virtual void buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent);

private:
	enum ButtonStates
	{
		BstNormal = 0,
		BstHover = 1 << 0,
		BstPushed = 1 << 1,
		BstToggled = 1 << 2
	};

	std::wstring m_text;
	Command m_command;
	uint32_t m_imageIndex;
	int32_t m_style;
	int32_t m_state;
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBarButton_H
