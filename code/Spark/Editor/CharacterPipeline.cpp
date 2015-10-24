#include "Core/Log/Log.h"
#include "Database/Instance.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Spark/ExternalData.h"
#include "Spark/SpriteData.h"
#include "Spark/TextData.h"
#include "Spark/Editor/CharacterPipeline.h"
#include "Spark/Script/ScriptComponentData.h"
#include "Spark/Sound/SoundComponentData.h"

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
	typeSet.insert(&type_of< CharacterData >());
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
	const CharacterData* character = mandatory_non_null_type_cast< const CharacterData* >(sourceAsset);

	if (const SpriteData* sprite = dynamic_type_cast< const SpriteData* >(character))
	{
		pipelineDepends->addDependency(sprite->m_shape, editor::PdfBuild | editor::PdfResource);

		for (RefArray< IComponentData >::const_iterator i = sprite->m_components.begin(); i != sprite->m_components.end(); ++i)
		{
			if (const ScriptComponentData* scriptComponent = dynamic_type_cast< const ScriptComponentData* >(*i))
				pipelineDepends->addDependency(scriptComponent->m_class, editor::PdfBuild);
			else if (const SoundComponentData* soundComponent = dynamic_type_cast< const SoundComponentData* >(*i))
			{
				for (SmallMap< std::wstring, resource::Id< sound::Sound > >::const_iterator j = soundComponent->m_sounds.begin(); j != soundComponent->m_sounds.end(); ++j)
					pipelineDepends->addDependency(j->second, editor::PdfBuild | editor::PdfResource);
			}
		}

		for (AlignedVector< SpriteData::NamedCharacter >::const_iterator i = sprite->m_dictionary.begin(); i != sprite->m_dictionary.end(); ++i)
			pipelineDepends->addDependency(i->character);

		for (AlignedVector< SpriteData::NamedCharacter >::const_iterator i = sprite->m_frame.begin(); i != sprite->m_frame.end(); ++i)
			pipelineDepends->addDependency(i->character);
	}
	else if (const TextData* text = dynamic_type_cast< const TextData* >(character))
	{
		pipelineDepends->addDependency(text->m_font, editor::PdfBuild | editor::PdfResource);
	}
	else if (const ExternalData* xternal = dynamic_type_cast< const ExternalData* >(character))
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
