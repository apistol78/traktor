#pragma once

#include "Core/Config.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace ui
	{

class IWidget;

/*! PathDialog interface.
 * \ingroup UI
 */
class IPathDialog
{
public:
	virtual ~IPathDialog() {}

	virtual bool create(IWidget* parent, const std::wstring& title) = 0;

	virtual void destroy() = 0;

	virtual DialogResult showModal(Path& outPath) = 0;
};

	}
}

