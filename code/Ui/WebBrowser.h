#ifndef traktor_ui_WebBrowser_H
#define traktor_ui_WebBrowser_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief WebBrowser
 * \ingroup UI
 */
class T_DLLCLASS WebBrowser : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& url);

	void navigate(const std::wstring& url);
};

	}
}

#endif	// traktor_ui_WebBrowser_H
