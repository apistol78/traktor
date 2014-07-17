#ifndef traktor_ui_custom_MiniButton_H
#define traktor_ui_custom_MiniButton_H

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

/*! \brief Mini button control.
 * \ingroup UIC
 */
class T_DLLCLASS MiniButton : public Widget
{
	T_RTTI_CLASS;
	
public:
	bool create(Widget* parent, const std::wstring& text);

	bool create(Widget* parent, Bitmap* image);

	void setImage(Bitmap* image);

	virtual Size getPreferedSize() const;

private:
	enum State
	{
		StReleased,
		StPushed
	};
	
	State m_state;
	Ref< Bitmap > m_image;
	
	void eventButtonDown(MouseButtonDownEvent* event);
	
	void eventButtonUp(MouseButtonUpEvent* event);
	
	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_MiniButton_H
