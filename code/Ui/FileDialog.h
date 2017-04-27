/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_FileDialog_H
#define traktor_ui_FileDialog_H

#include <vector>
#include "Core/Io/Path.h"
#include "Ui/EventSubject.h"

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

class Widget;
class IFileDialog;

/*! \brief File dialog.
 * \ingroup UI
 */
class T_DLLCLASS FileDialog : public EventSubject
{
	T_RTTI_CLASS;

public:
	FileDialog();

	virtual ~FileDialog();

	bool create(Widget* parent, const std::wstring& title, const std::wstring& filters, bool save = false);

	void destroy();

	int showModal(Path& outPath);

	int showModal(std::vector< Path >& outPaths);

private:
	IFileDialog* m_fileDialog;
};

	}
}

#endif	// traktor_ui_FileDialog_H
