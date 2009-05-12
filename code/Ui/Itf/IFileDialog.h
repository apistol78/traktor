#ifndef traktor_ui_IFileDialog_H
#define traktor_ui_IFileDialog_H

#include <vector>
#include "Core/Config.h"
#include "Core/Io/Path.h"

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

/*! \brief FileDialog interface.
 * \ingroup UI
 */
class T_DLLCLASS IFileDialog
{
public:
	virtual ~IFileDialog() {}

	virtual bool create(IWidget* parent, const std::wstring& title, const std::wstring& filters, bool save) = 0;

	virtual void destroy() = 0;

	virtual int showModal(Path& outPath) = 0;

	virtual int showModal(std::vector< Path >& outPaths) = 0;
};

	}
}

#endif	// traktor_ui_IFileDialog_H
