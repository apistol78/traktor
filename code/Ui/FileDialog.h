/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Ref.h"
#include "Core/Io/Path.h"
#include "Ui/ConfigDialog.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class Container;
class Edit;
class GridView;
class IBitmap;

/*! File dialog.
 * \ingroup UI
 */
class T_DLLCLASS FileDialog : public ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(
		Widget* parent,
		const std::wstring& key,
		const std::wstring& title,
		const std::wstring& filters,
		const std::wstring& defaultPath = L"",
		bool save = false
	);

	virtual void destroy() override;

	DialogResult showModal(Path& outPath);

	DialogResult showModal(std::vector< Path >& outPaths);

	DialogResult showModalThenDestroy(Path& outPath)
	{
		const DialogResult result = showModal(outPath);
		destroy();
		return result;
	}

	DialogResult showModalThenDestroy(std::vector< Path >& outPaths)
	{
		const DialogResult result = showModal(outPaths);
		destroy();
		return result;
	}

private:
	Ref< Container > m_containerPath;
	Ref< GridView > m_gridFiles;
	Ref< Edit > m_editFileName;
	Ref< IBitmap > m_bitmapDirectory;
	Ref< IBitmap > m_bitmapFile;
	std::wstring m_key;
	Path m_defaultPath;
	Path m_currentPath;

	void updatePath();

	void updateFiles();
};

}
