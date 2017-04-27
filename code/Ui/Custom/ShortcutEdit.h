/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_ShortcutEdit_H
#define traktor_ui_custom_ShortcutEdit_H

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
		namespace custom
		{

/*! \brief Shortcut edit control.
 * \ingroup UIC
 */
class T_DLLCLASS ShortcutEdit : public Widget
{
	T_RTTI_CLASS;

public:
	ShortcutEdit();

	bool create(Widget* parent, int32_t keyState, VirtualKey virtualKey, int style = WsNone);

	virtual std::wstring getText() const T_OVERRIDE;

	virtual Size getPreferedSize() const T_OVERRIDE;

	void set(int32_t keyState, VirtualKey virtualKey);

	int32_t getKeyState() const;

	VirtualKey getVirtualKey() const;

private:
	int32_t m_keyState;
	VirtualKey m_virtualKey;

	void eventKeyDown(KeyDownEvent* event);

	void eventPaint(PaintEvent* event);

	void eventFocus(FocusEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ShortcutEdit_H
