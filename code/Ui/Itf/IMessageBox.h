#ifndef traktor_ui_IMessageBox_H
#define traktor_ui_IMessageBox_H

#include <string>
#include "Core/Config.h"

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

class IWidget;

/*! \brief MessageBox interface.
 * \ingroup UI
 */
class T_DLLCLASS IMessageBox
{
public:
	virtual bool create(IWidget* parent, const std::wstring& message, const std::wstring& caption, int style) = 0;

	virtual void destroy() = 0;

	virtual int showModal() = 0;
};

	}
}

#endif	// traktor_ui_IMessageBox_H
