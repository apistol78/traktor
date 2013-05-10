#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Editor/Assets.h"
#include "Editor/AssetsPipeline.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/VirtualAsset.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.AssetsPipeline", 1, AssetsPipeline, IPipeline)

bool AssetsPipeline::create(const IPipelineSettings* settings)
{
	return true;
}

void AssetsPipeline::destroy()
{
}

TypeInfoSet AssetsPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Assets >());
	typeSet.insert(&type_of< VirtualAsset >());
	return typeSet;
}

bool AssetsPipeline::buildDependencies(
	IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const Assets* assets = dynamic_type_cast< const Assets* >(sourceAsset))
	{
		for (std::vector< Guid >::const_iterator i = assets->m_dependencies.begin(); i != assets->m_dependencies.end(); ++i)
			pipelineDepends->addDependency(*i, editor::PdfBuild);
	}
	else if (const VirtualAsset* virtualAsset = dynamic_type_cast< const VirtualAsset* >(sourceAsset))
	{
		Ref< db::Instance > virtualSourceInstance = pipelineDepends->getSourceDatabase()->getInstance(virtualAsset->getSourceInstance());
		Ref< db::Instance > virtualPlaceholderInstance = pipelineDepends->getSourceDatabase()->getInstance(virtualAsset->getPlaceholderInstance());

		if (!virtualSourceInstance || !virtualPlaceholderInstance)
			return false;

		Ref< ISerializable > object = virtualSourceInstance->getObject< ISerializable >();
		if (!object)
			return false;

		pipelineDepends->addDependency(
			object,
			virtualPlaceholderInstance->getPath(),
			virtualPlaceholderInstance->getGuid(),
			editor::PdfBuild
		);
	}
	return true;
}

bool AssetsPipeline::buildOutput(
	IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	return true;
}

Ref< ISerializable > AssetsPipeline::buildOutput(
	IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
