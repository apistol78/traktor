#pragma once

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

/*! Table layout.
 * \ingroup UI
 */
class T_DLLCLASS TableLayout : public Layout
{
	T_RTTI_CLASS;

public:
	TableLayout(const std::wstring& cdef, const std::wstring& rdef, int margin, int pad);

	virtual bool fit(Widget* widget, const Size& bounds, Size& result) override;

	virtual void update(Widget* widget) override;

private:
	Size m_margin;
	Size m_pad;
	std::vector< int32_t > m_cdef;
	std::vector< int32_t > m_rdef;
};

	}
}

