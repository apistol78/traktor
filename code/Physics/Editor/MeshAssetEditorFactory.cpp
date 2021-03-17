#include "Core/Serialization/DeepClone.h"
#include "Physics/Editor/MeshAsset.h"
#include "Physics/Editor/MeshAssetEditor.h"
#include "Physics/Editor/MeshAssetEditorFactory.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshAssetEditorFactory", 0, MeshAssetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet MeshAssetEditorFactory::getEditableTypes() const
{
	return makeTypeInfoSet< MeshAsset >();
}

bool MeshAssetEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IObjectEditor > MeshAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new MeshAssetEditor(editor);
}

void MeshAssetEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

Ref< ISerializable > MeshAssetEditorFactory::cloneAsset(const ISerializable* asset) const
{
	return DeepClone(asset).create();
}

	}
}
