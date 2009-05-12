#ifndef traktor_ui_TabPage_H
#define traktor_ui_TabPage_H

#include "Core/Heap/Ref.h"
#include "Ui/Container.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Tab;

/*! \brief Tab page.
 * \ingroup UI
 */
class T_DLLCLASS TabPage : public Container
{
	T_RTTI_CLASS(TabPage)

public:
	bool create(Tab* tab, const std::wstring& text, int imageIndex, Layout* layout);

	bool create(Tab* tab, const std::wstring& text, Layout* layout);

	void setActive();

	bool isActive() const;

	Tab* getTab();

	int getImageIndex() const;

private:
	Ref< Tab > m_tab;
	int m_imageIndex;
};

	}
}

#endif	// traktor_ui_TabPage_H
