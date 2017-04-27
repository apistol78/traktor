/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/NotificationIcon.h"
#include "Ui/Widget.h"
#include "Ui/Itf/INotificationIcon.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.NotificationIcon", NotificationIcon, EventSubject)

NotificationIcon::NotificationIcon()
:	m_ni(0)
{
}

NotificationIcon::~NotificationIcon()
{
	T_ASSERT_M (!m_ni, L"NotificationIcon not destroyed");
}

bool NotificationIcon::create(const std::wstring& text, Bitmap* image)
{
	if (!image || !image->getSystemBitmap())
		return false;

	m_ni = Application::getInstance()->getWidgetFactory()->createNotificationIcon(this);
	if (!m_ni)
	{
		log::error << L"Failed to create native widget peer (NotificationIcon)" << Endl;
		return false;
	}

	if (!m_ni->create(text, image->getSystemBitmap()))
		return false;

	return true;
}

void NotificationIcon::destroy()
{
	safeDestroy(m_ni);
}

void NotificationIcon::setImage(Bitmap* image)
{
	m_ni->setImage(image->getSystemBitmap());
}

	}
}
