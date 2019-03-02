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

namespace traktor
{
	namespace ui
	{

class Container;
class GridView;
class IBitmap;

/*! \brief File dialog.
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
		bool save = false
	);

	void destroy();

	int32_t showModal(Path& outPath);

	int32_t showModal(std::vector< Path >& outPaths);

private:
	Ref< Container > m_containerPath;
	Ref< GridView > m_gridFiles;
	Ref< IBitmap > m_bitmapDirectory;
	Ref< IBitmap > m_bitmapFile;
	std::wstring m_key;
	Path m_currentPath;

	void updatePath();

	void updateFiles();
};

	}
}
