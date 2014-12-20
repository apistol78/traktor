#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Database/Instance.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "World/Editor/PostProcessPipeline.h"
#include "World/Editor/PostProcess/IPostProcessStepFacade.h"
#include "World/PostProcess/PostProcessDefineTarget.h"
#include "World/PostProcess/PostProcessDefineTexture.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcessStepBlur.h"
#include "World/PostProcess/PostProcessStepBokeh.h"
#include "World/PostProcess/PostProcessStepChain.h"
#include "World/PostProcess/PostProcessStepDiscardTarget.h"
#include "World/PostProcess/PostProcessStepGodRay.h"
#include "World/PostProcess/PostProcessStepGrain.h"
#include "World/PostProcess/PostProcessStepLensDirt.h"
#include "World/PostProcess/PostProcessStepLuminance.h"
#include "World/PostProcess/PostProcessStepRepeat.h"
#include "World/PostProcess/PostProcessStepSimple.h"
#include "World/PostProcess/PostProcessStepSsao.h"
#include "World/PostProcess/PostProcessStepSmProj.h"
#include "World/PostProcess/PostProcessStepTemporal.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

void gatherSources(const RefArray< PostProcessStep >& steps, std::vector< std::wstring >& outSources);

Ref< IPostProcessStepFacade > createFacade(const PostProcessStep* step)
{
	const TypeInfo* facadeType = TypeInfo::find(std::wstring(type_name(step)) + L"Facade");
	if (facadeType)
		return checked_type_cast< IPostProcessStepFacade* >(facadeType->createInstance());
	else
		return 0;
}

void gatherSources(const PostProcessStep* step, std::vector< std::wstring >& outSources)
{
	Ref< IPostProcessStepFacade > facade = createFacade(step);
	if (facade)
	{
		facade->getSources(step, outSources);

		RefArray< PostProcessStep > children;
		facade->getChildren(step, children);
		gatherSources(children, outSources);
	}
}

void gatherSources(const RefArray< PostProcessStep >& steps, std::vector< std::wstring >& outSources)
{
	for (RefArray< PostProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
		gatherSources(*i, outSources);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessPipeline", 2, PostProcessPipeline, editor::IPipeline)

bool PostProcessPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void PostProcessPipeline::destroy()
{
}

TypeInfoSet PostProcessPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< PostProcessSettings >());
	return typeSet;
}

bool PostProcessPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const PostProcessSettings* postProcessSettings = checked_type_cast< const PostProcessSettings* >(sourceAsset);
	T_FATAL_ASSERT (postProcessSettings);

	const RefArray< PostProcessDefine >& definitions = postProcessSettings->getDefinitions();
	for (RefArray< PostProcessDefine >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
	{
		if (const PostProcessDefineTexture* defineTexture = dynamic_type_cast< const PostProcessDefineTexture* >(*i))
			pipelineDepends->addDependency(defineTexture->getTexture(), editor::PdfBuild | editor::PdfResource);
	}
	
	RefArray< PostProcessStep > ss;

	const RefArray< PostProcessStep >& steps = postProcessSettings->getSteps();
	for (RefArray< PostProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
		ss.push_back(*i);

	while (!ss.empty())
	{
		Ref< PostProcessStep > step = ss.front(); ss.pop_front();

		if (const PostProcessStepBlur* stepBlur = dynamic_type_cast< const PostProcessStepBlur* >(step))
			pipelineDepends->addDependency(stepBlur->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const PostProcessStepBokeh* stepBokeh = dynamic_type_cast< const PostProcessStepBokeh* >(step))
			pipelineDepends->addDependency(stepBokeh->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const PostProcessStepChain* stepChain = dynamic_type_cast< const PostProcessStepChain* >(step))
		{
			const RefArray< PostProcessStep >& steps = stepChain->getSteps();
			for (RefArray< PostProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
				ss.push_back(*i);
		}
		else if (const PostProcessStepGodRay* stepGodRay = dynamic_type_cast< const PostProcessStepGodRay* >(step))
			pipelineDepends->addDependency(stepGodRay->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const PostProcessStepGrain* stepGrain = dynamic_type_cast< const PostProcessStepGrain* >(step))
			pipelineDepends->addDependency(stepGrain->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const PostProcessStepLensDirt* stepLensDirt = dynamic_type_cast< const PostProcessStepLensDirt* >(step))
			pipelineDepends->addDependency(stepLensDirt->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const PostProcessStepLuminance* stepLuminance = dynamic_type_cast< const PostProcessStepLuminance* >(step))
			pipelineDepends->addDependency(stepLuminance->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const PostProcessStepRepeat* stepRepeat = dynamic_type_cast< const PostProcessStepRepeat* >(step))
			ss.push_back(stepRepeat->getStep());
		else if (const PostProcessStepSimple* stepSimple = dynamic_type_cast< const PostProcessStepSimple* >(step))
			pipelineDepends->addDependency(stepSimple->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const PostProcessStepSsao* stepSsao = dynamic_type_cast< const PostProcessStepSsao* >(step))
			pipelineDepends->addDependency(stepSsao->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const PostProcessStepSmProj* stepSmProj = dynamic_type_cast< const PostProcessStepSmProj* >(step))
			pipelineDepends->addDependency(stepSmProj->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const PostProcessStepTemporal* stepTemporal = dynamic_type_cast< const PostProcessStepTemporal* >(step))
			pipelineDepends->addDependency(stepTemporal->getShader(), editor::PdfBuild | editor::PdfResource);
	}

	return true;
}

bool PostProcessPipeline::buildOutput(
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
	Ref< PostProcessSettings > pp = DeepClone(sourceAsset).create< PostProcessSettings >();
	if (!pp)
		return false;

	std::set< std::wstring > targets;

	// Get all user defined, non-persistent, targets.
	const RefArray< PostProcessDefine >& definitions = pp->getDefinitions();
	for (RefArray< PostProcessDefine >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
	{
		if (const PostProcessDefineTarget* defineTarget = dynamic_type_cast< const PostProcessDefineTarget* >(*i))
		{
			if (!defineTarget->persistent())
				targets.insert(defineTarget->getId());
		}
	}

	// Copy steps, insert discard target steps when a user defined target is no longer used
	// to improve target pooling.
	RefArray< PostProcessStep > outputSteps;
	const RefArray< PostProcessStep >& steps = pp->getSteps();
	for (RefArray< PostProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
	{
		Ref< IPostProcessStepFacade > facade = createFacade(*i);
		if (!facade)
		{
			log::error << L"No facade found for post process step " << type_name(*i) << Endl;
			return false;
		}

		outputSteps.push_back(*i);

		// Get sources which are still to be used.
		std::vector< std::wstring > toBeUsedSources;
		for (RefArray< PostProcessStep >::const_iterator j = i + 1; j != steps.end(); ++j)
			gatherSources(*j, toBeUsedSources);

		// Discard targets which will no longer be used.
		for (std::set< std::wstring >::iterator j = targets.begin(); j != targets.end(); )
		{
			if (std::find(toBeUsedSources.begin(), toBeUsedSources.end(), *j) == toBeUsedSources.end())
			{
				log::info << L"Inserting discard target step \"" << *j << L"\"" << Endl;
				outputSteps.push_back(new PostProcessStepDiscardTarget(*j));
				targets.erase(j); j = targets.begin();
			}
			else
				++j;
		}
	}

	// Replace steps which include discard steps.
	pp->setSteps(outputSteps);

	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(outputPath, outputGuid);
	if (!outputInstance)
		return false;

	outputInstance->setObject(pp);

	if (!outputInstance->commit())
		return false;

	return true;
}

Ref< ISerializable > PostProcessPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
