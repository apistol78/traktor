#ifndef traktor_ui_TableLayout_H
#define traktor_ui_TableLayout_H

#include <string>
#include <vector>
#include "Ui/Layout.h"
#include "Ui/Size.h"

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

/*! \brief Table layout.
 * \ingroup UI
 */
class T_DLLCLASS TableLayout : public Layout
{
	T_RTTI_CLASS;

public:
	TableLayout(const std::wstring& cdef, const std::wstring& rdef, int margin, int pad);
	
	virtual bool fit(Widget* widget, const Size& bounds, Size& result) T_OVERRIDE;
	
	virtual void update(Widget* widget) T_OVERRIDE;

private:
	Size m_margin;
	Size m_pad;
	std::vector< int > m_cdef;
	std::vector< int > m_rdef;
};
	
	}
}

#endif	// traktor_ui_TableLayout_H
