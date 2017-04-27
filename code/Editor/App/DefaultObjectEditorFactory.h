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
	virtual const TypeInfoSet getEditableTypes() const T_OVERRIDE T_FINAL;

	virtual bool needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const T_OVERRIDE T_FINAL;

	virtual Ref< IObjectEditor > createObjectEditor(IEditor* editor) const T_OVERRIDE T_FINAL;

	virtual void getCommands(std::list< ui::Command >& outCommands) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_editor_DefaultObjectEditorFactory_H
