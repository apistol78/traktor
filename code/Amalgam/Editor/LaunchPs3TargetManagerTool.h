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
	virtual std::wstring getDescription() const override final;

	virtual Ref< ui::IBitmap > getIcon() const override final;

	virtual bool needOutputResources(std::set< Guid >& outDependencies) const override final;

	virtual bool launch(ui::Widget* parent, editor::IEditor* amalgam, const std::wstring& param) override final;
};

	}
}

#endif	// traktor_amalgam_LaunchPs3TargetManagerTool_H

