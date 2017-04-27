/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_MessageBox_H
#define traktor_ui_MessageBox_H

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

// Remove MS definition.
#if defined(MessageBox)
#	undef MessageBox
#endif

class Widget;
class IMessageBox;

/*! \brief Message box.
 * \ingroup UI
 */
class T_DLLCLASS MessageBox : public EventSubject
{
	T_RTTI_CLASS;

public:
	MessageBox();

	virtual ~MessageBox();

	bool create(Widget* parent, const std::wstring& message, const std::wstring& caption, int style);

	void destroy();

	int showModal();

	static int show(Widget* parent, const std::wstring& message, const std::wstring& caption, int style);

	static int show(const std::wstring& message, const std::wstring& caption, int style);

private:
	IMessageBox* m_messageBox;
};

	}
}

#endif	// traktor_ui_MessageBox_H
