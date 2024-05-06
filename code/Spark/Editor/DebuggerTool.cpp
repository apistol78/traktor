/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Spark/Editor/DebuggerTool.h"
#include "I18N/Text.h"
#include "Ui/StyleBitmap.h"

namespace traktor
{
	namespace spark
	{


T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.DebuggerTool", 0, DebuggerTool, IEditorTool)

std::wstring DebuggerTool::getDescription() const
{
	return i18n::Text(L"FLASH_DEBUGGER_TOOL");
}

Ref< ui::IBitmap > DebuggerTool::getIcon() const
{
	return nullptr;
}

bool DebuggerTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool DebuggerTool::launch(ui::Widget* parent, editor::IEditor* editor, const PropertyGroup* param)
{
	//// If debugger dialog is already created then we only need to show it.
	//if (m_debuggerDialog)
	//{
	//	m_debuggerDialog->show();
	//	return true;
	//}

	//// Create dialog and register ourself for close events.
	//m_debuggerDialog = new FlashDebuggerDialog(editor);
	//if (m_debuggerDialog->create(parent))
	//{
	//	m_debuggerDialog->show();
	//	m_debuggerDialog->addEventHandler< ui::CloseEvent >(this, &DebuggerTool::eventCloseDialog);
	//}
	//else
	//	safeDestroy(m_debuggerDialog);

	return true;
}

void DebuggerTool::eventCloseDialog(ui::CloseEvent* event)
{
	//safeDestroy(m_debuggerDialog);
}

	}
}
