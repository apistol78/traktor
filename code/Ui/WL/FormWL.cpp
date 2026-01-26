/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/WL/FormWL.h"

namespace traktor::ui
{

FormWL::FormWL(Context* context, EventSubject* owner)
:	WidgetWLImpl< IForm >(context, owner)
{
}

bool FormWL::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	wl_surface* clientSurface = m_context->createSurface(width, height);

	wl_shell_surface* shellSurface = wl_shell_get_shell_surface(
		m_context->getShell(),
		clientSurface
	);
	wl_shell_surface_set_toplevel(shellSurface);

	return true;
}

void FormWL::setIcon(ISystemBitmap* icon)
{
}

void FormWL::maximize()
{
}

void FormWL::minimize()
{
}

void FormWL::restore()
{
}

bool FormWL::isMaximized() const
{
	return false;
}

bool FormWL::isMinimized() const
{
	return false;
}

void FormWL::hideProgress()
{
}

void FormWL::showProgress(int32_t current, int32_t total)
{
}

}
