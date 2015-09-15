#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Spark/External.h"
#include "Spark/ScriptComponent.h"
#include "Spark/Sprite.h"
#include "Spark/Editor/CharacterPipeline.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.CharacterPipeline", 0, CharacterPipeline, editor::IPipeline)

CharacterPipeline::CharacterPipeline()
{
}

bool CharacterPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void CharacterPipeline::destroy()
{
}

TypeInfoSet CharacterPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Character >());
	return typeSet;
}

bool CharacterPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const Character* character = mandatory_non_null_type_cast< const Character* >(sourceAsset);

	// Add component resource dependencies.
	for (RefArray< IComponent >::const_iterator i = character->m_components.begin(); i != character->m_components.end(); ++i)
	{
		if (const ScriptComponent* scriptComponent = dynamic_type_cast< const ScriptComponent* >(*i))
		{
			pipelineDepends->addDependency(scriptComponent->m_class, editor::PdfBuild | editor::PdfResource);
		}
	}

	// Add specific character type dependencies.
	if (const Sprite* sprite = dynamic_type_cast< const Sprite* >(character))
	{
		pipelineDepends->addDependency(sprite->m_shape, editor::PdfBuild | editor::PdfResource);
		for (std::map< std::wstring, Ref< Character > >::const_iterator i = sprite->m_characters.begin(); i != sprite->m_characters.end(); ++i)
			pipelineDepends->addDependency(i->second);
	}
	else if (const External* xternal = dynamic_type_cast< const External* >(character))
	{
		pipelineDepends->addDependency(xternal->m_reference, editor::PdfBuild | editor::PdfResource);
	}

	return true;
}

bool CharacterPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
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
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
	{
		log::error << L"Character pipeline failed; unable to create output instance." << Endl;
		return false;
	}

	outputInstance->setObject(sourceAsset);

	if (!outputInstance->commit())
	{
		log::error << L"Character pipeline failed; unable to commit output instance." << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > CharacterPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
