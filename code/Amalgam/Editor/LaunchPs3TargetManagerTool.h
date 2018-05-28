/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_LaunchPs3TargetManagerTool_H
#define traktor_amalgam_LaunchPs3TargetManagerTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace amalgam
	{

class LaunchPs3TargetManagerTool : public editor::IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual Ref< ui::IBitmap > getIcon() const T_OVERRIDE T_FINAL;

	virtual bool needOutputResources(std::set< Guid >& outDependencies) const T_OVERRIDE T_FINAL;

	virtual bool launch(ui::Widget* parent, editor::IEditor* amalgam, const std::wstring& param) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_amalgam_LaunchPs3TargetManagerTool_H

