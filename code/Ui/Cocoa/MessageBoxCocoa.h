/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_MessageBoxCocoa_H
#define traktor_ui_MessageBoxCocoa_H

#include "Ui/Itf/IMessageBox.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class MessageBoxCocoa : public IMessageBox
{
public:
	MessageBoxCocoa(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& message, const std::wstring& caption, int style) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual int showModal() T_OVERRIDE T_FINAL;

private:
	std::wstring m_message;
	std::wstring m_caption;
};

	}
}

#endif	// traktor_ui_MessageBoxCocoa_H
