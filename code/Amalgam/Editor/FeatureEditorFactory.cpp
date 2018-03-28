/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Editor/Deploy/Feature.h"
#include "Amalgam/Editor/FeatureEditor.h"
#include "Amalgam/Editor/FeatureEditorFactory.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.FeatureEditorFactory", 0, FeatureEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet FeatureEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Feature >());
	return typeSet;
}

bool FeatureEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IObjectEditor > FeatureEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new FeatureEditor(editor);
}

void FeatureEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
