/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/PathDialog.h"
#include "Ui/Widget.h"
#include "Ui/Itf/IPathDialog.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PathDialog", PathDialog, EventSubject)

PathDialog::~PathDialog()
{
	T_ASSERT_M (!m_pathDialog, L"PathDialog not destroyed");
}

bool PathDialog::create(Widget* parent, const std::wstring& title)
{
	m_pathDialog = Application::getInstance()->getWidgetFactory()->createPathDialog(this);
	if (!m_pathDialog)
	{
		log::error << L"Failed to create native widget peer (PathDialog)" << Endl;
		return false;
	}

	if (!m_pathDialog->create(parent ? parent->getIWidget() : nullptr, title))
		return false;

	return true;
}

void PathDialog::destroy()
{
	if (m_pathDialog)
	{
		m_pathDialog->destroy();
		m_pathDialog = nullptr;
	}
}

DialogResult PathDialog::showModal(Path& outPath)
{
	T_ASSERT(m_pathDialog);
	return m_pathDialog->showModal(outPath);
}

}
