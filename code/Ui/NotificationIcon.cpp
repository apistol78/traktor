/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Ui/Application.h"
#include "Ui/IBitmap.h"
#include "Ui/NotificationIcon.h"
#include "Ui/Widget.h"
#include "Ui/Itf/INotificationIcon.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.NotificationIcon", NotificationIcon, EventSubject)

NotificationIcon::~NotificationIcon()
{
	T_ASSERT_M(!m_ni, L"NotificationIcon not destroyed");
}

bool NotificationIcon::create(const std::wstring& text, IBitmap* image)
{
	if (!image || !image->getSystemBitmap(nullptr))
		return false;

	m_ni = Application::getInstance()->getWidgetFactory()->createNotificationIcon(this);
	if (!m_ni)
	{
		log::error << L"Failed to create native widget peer (NotificationIcon)" << Endl;
		return false;
	}

	if (!m_ni->create(text, image->getSystemBitmap(nullptr)))
		return false;

	return true;
}

void NotificationIcon::destroy()
{
	safeDestroy(m_ni);
}

void NotificationIcon::setImage(IBitmap* image)
{
	m_ni->setImage(image->getSystemBitmap(nullptr));
}

}
