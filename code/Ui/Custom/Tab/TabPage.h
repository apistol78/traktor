#ifndef traktor_ui_custom_TabPage_H
#define traktor_ui_custom_TabPage_H

#include "Ui/Container.h"

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

class Tab;

/*! \brief Tab page.
 * \ingroup UIC
 */
class T_DLLCLASS TabPage : public Container
{
	T_RTTI_CLASS;

public:
	bool create(Tab* tab, const std::wstring& text, Layout* layout);

	void setActive();

	bool isActive() const;

	Ref< Tab > getTab();

private:
	Ref< Tab > m_tab;

	void eventPaint(PaintEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_TabPage_H
