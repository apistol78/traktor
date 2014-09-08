#ifndef traktor_ui_IPathDialog_H
#define traktor_ui_IPathDialog_H

#include "Core/Config.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace ui
	{

class IWidget;

/*! \brief PathDialog interface.
 * \ingroup UI
 */
class IPathDialog
{
public:
	virtual ~IPathDialog() {}

	virtual bool create(IWidget* parent, const std::wstring& title) = 0;

	virtual void destroy() = 0;

	virtual int showModal(Path& outPath) = 0;
};

	}
}

#endif	// traktor_ui_IPathDialog_H
