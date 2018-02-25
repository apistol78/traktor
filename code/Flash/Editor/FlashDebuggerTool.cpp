/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Flash/Editor/FlashDebuggerTool.h"
#include "I18N/Text.h"
#include "Ui/StyleBitmap.h"

namespace traktor
{
	namespace flash
	{


T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashDebuggerTool", 0, FlashDebuggerTool, IEditorTool)

std::wstring FlashDebuggerTool::getDescription() const
{
	return i18n::Text(L"FLASH_DEBUGGER_TOOL");
}

Ref< ui::IBitmap > FlashDebuggerTool::getIcon() const
{
	return new ui::StyleBitmap(L"Flash.Tool.Debugger");
}

bool FlashDebuggerTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool FlashDebuggerTool::launch(ui::Widget* parent, editor::IEditor* editor, const std::wstring& param)
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
	//	m_debuggerDialog->addEventHandler< ui::CloseEvent >(this, &FlashDebuggerTool::eventCloseDialog);
	//}
	//else
	//	safeDestroy(m_debuggerDialog);

	return true;
}

void FlashDebuggerTool::eventCloseDialog(ui::CloseEvent* event)
{
	//safeDestroy(m_debuggerDialog);
}

	}
}
