/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

#include <vector>
#include "Core/Io/Path.h"
#include "Ui/ConfigDialog.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Container;

		namespace custom
		{

class GridView;

/*! \brief File dialog.
 * \ingroup UIC
 */
class T_DLLCLASS FileDialog : public ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& title, const std::wstring& filters, bool save = false);

	void destroy();

	int showModal(Path& outPath);

	int showModal(std::vector< Path >& outPaths);

private:
	Ref< Container > m_containerPath;
	Ref< custom::GridView > m_gridFiles;
	Path m_currentPath;

	void updatePath();

	void updateFiles();	
};

		}
	}
}
