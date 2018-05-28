/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Editor/LaunchPs3TargetManagerTool.h"
//#include "Amalgam/Editor/Deploy/Feature.h"
//#include "Core/Io/Path.h"
//#include "Core/Log/Log.h"
//#include "Core/Misc/String.h"
//#include "Core/Settings/PropertyGroup.h"
//#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/OS.h"
//#include "Database/Database.h"
//#include "Database/Group.h"
//#include "Database/Instance.h"
//#include "Database/Traverse.h"
//#include "Editor/IEditor.h"
//#include "I18N/Text.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.LaunchPs3TargetManagerTool", 0, LaunchPs3TargetManagerTool, IEditorTool)

std::wstring LaunchPs3TargetManagerTool::getDescription() const
{
	return L"Launch PS3 TargetManager...";
}

Ref< ui::IBitmap > LaunchPs3TargetManagerTool::getIcon() const
{
	return 0;
}

bool LaunchPs3TargetManagerTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool LaunchPs3TargetManagerTool::launch(ui::Widget* parent, editor::IEditor* amalgam, const std::wstring& param)
{
	return OS::getInstance().execute(L"\"$(SN_PS3_PATH)/bin/ps3tm.exe\"", L"$(SN_PS3_PATH)/bin", 0, false, false, false) != 0;
}

	}
}
