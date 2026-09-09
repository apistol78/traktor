/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/IEditorPage.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.IEditorPage", IEditorPage, Object)

void IEditorPage::handleDatabaseEvents(AlignedVector< std::pair< db::Database*, Guid > >& events)
{
	for (auto event : events)
		handleDatabaseEvent(event.first, event.second);
}

void IEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

}
