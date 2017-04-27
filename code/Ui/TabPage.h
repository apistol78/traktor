/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_TabPage_H
#define traktor_ui_TabPage_H

#include "Ui/Container.h"

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

class Tab;

/*! \brief Tab page.
 * \ingroup UI
 */
class T_DLLCLASS TabPage : public Container
{
	T_RTTI_CLASS;

public:
	bool create(Tab* tab, const std::wstring& text, int imageIndex, Layout* layout);

	bool create(Tab* tab, const std::wstring& text, Layout* layout);

	void setActive();

	bool isActive() const;

	Ref< Tab > getTab();

	int getImageIndex() const;

private:
	Ref< Tab > m_tab;
	int m_imageIndex;
};

	}
}

#endif	// traktor_ui_TabPage_H
