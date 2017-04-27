/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_IFileDialog_H
#define traktor_ui_IFileDialog_H

#include <vector>
#include "Core/Config.h"
#include "Core/Io/Path.h"

namespace traktor
{
	namespace ui
	{

class IWidget;

/*! \brief FileDialog interface.
 * \ingroup UI
 */
class IFileDialog
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
