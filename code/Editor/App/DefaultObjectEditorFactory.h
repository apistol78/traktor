/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_DefaultObjectEditorFactory_H
#define traktor_editor_DefaultObjectEditorFactory_H

#include "Editor/IObjectEditorFactory.h"

namespace traktor
{
	namespace editor
	{

class DefaultObjectEditorFactory : public IObjectEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const override final;

	virtual bool needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const override final;

	virtual Ref< IObjectEditor > createObjectEditor(IEditor* editor) const override final;

	virtual void getCommands(std::list< ui::Command >& outCommands) const override final;
};

	}
}

#endif	// traktor_editor_DefaultObjectEditorFactory_H
