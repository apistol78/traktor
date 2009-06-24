#include "Spray/Editor/EffectPipeline.h"
#include "Spray/Effect.h"
#include "Spray/EffectLayer.h"
#include "Spray/Emitter.h"
#include "Editor/PipelineManager.h"
#include "Database/Instance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.spray.EffectPipeline", EffectPipeline, editor::IPipeline)

bool EffectPipeline::create(const editor::Settings* settings)
{
	return true;
}

void EffectPipeline::destroy()
{
}

uint32_t EffectPipeline::getVersion() const
{
	return 1;
}

TypeSet EffectPipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Effect >());
	return typeSet;
}

bool EffectPipeline::buildDependencies(
	editor::PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const Effect* effect = checked_type_cast< const Effect* >(sourceAsset);

	const RefArray< EffectLayer >& layers = effect->getLayers();
	for (RefArray< EffectLayer >::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		const Emitter* emitter = (*i)->getEmitter();
		if (emitter)
			pipelineManager->addDependency(emitter->getShader().getGuid());
	}

	return true;
}

bool EffectPipeline::buildOutput(
	editor::PipelineManager* pipelineManager,
	const Serializable* sourceAsset,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< db::Instance > instance = pipelineManager->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
		return false;

	instance->setObject(sourceAsset);

	return instance->commit();
}

	}
}
