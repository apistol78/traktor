#pragma once

#include "Ui/ConfigDialog.h"

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

// Remove MS definition.
#if defined(MessageBox)
#	undef MessageBox
#endif

/*! Message box.
 * \ingroup UI
 */
class T_DLLCLASS MessageBox : public ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& message, const std::wstring& caption, int style);

	static DialogResult show(Widget* parent, const std::wstring& message, const std::wstring& caption, int style);

	static DialogResult show(const std::wstring& message, const std::wstring& caption, int style);
};

	}
}

