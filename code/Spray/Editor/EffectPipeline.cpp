#include "Spray/Editor/EffectPipeline.h"
#include "Spray/Effect.h"
#include "Spray/EffectLayer.h"
#include "Spray/Emitter.h"
#include "Spray/Sources/PointSetSource.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Database/Instance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectPipeline", EffectPipeline, editor::IPipeline)

bool EffectPipeline::create(const editor::IPipelineSettings* settings)
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

TypeInfoSet EffectPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Effect >());
	return typeSet;
}

bool EffectPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	const Effect* effect = checked_type_cast< const Effect* >(sourceAsset);

	const RefArray< EffectLayer >& layers = effect->getLayers();
	for (RefArray< EffectLayer >::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		const Emitter* emitter = (*i)->getEmitter();
		if (emitter)
		{
			pipelineDepends->addDependency(emitter->getShader().getGuid(), true);

			const PointSetSource* pointSetSource = dynamic_type_cast< const PointSetSource* >(emitter->getSource());
			if (pointSetSource)
				pipelineDepends->addDependency(pointSetSource->getPointSet().getGuid(), true);
		}
	}

	return true;
}

bool EffectPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
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
