#pragma once

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

	virtual Ref< ISerializable > cloneAsset(const ISerializable* asset) const override final;
};

	}
}

