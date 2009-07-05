#ifndef traktor_ui_custom_ShortcutEdit_H
#define traktor_ui_custom_ShortcutEdit_H

#include "Core/Heap/Ref.h"
#include "Ui/Widget.h"

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

/*! \brief Shortcut edit control.
 * \ingroup UIC
 */
class T_DLLCLASS ShortcutEdit : public Widget
{
	T_RTTI_CLASS(ShortcutEdit)

public:
	ShortcutEdit();

	bool create(Widget* parent, int32_t keyState, int32_t keyCode, int style = WsNone);

	virtual std::wstring getText() const;

	virtual Size getPreferedSize() const;

	void set(int32_t keyState, int32_t keyCode);

	int32_t getKeyState() const;

	int32_t getKeyCode() const;

	void addChangeEventHandler(EventHandler* eventHandler);

private:
	int32_t m_keyState;
	int32_t m_keyCode;

	void eventKeyDown(Event* event);

	void eventPaint(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ShortcutEdit_H
