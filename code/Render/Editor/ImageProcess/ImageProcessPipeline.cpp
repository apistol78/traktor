#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Database/Instance.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineBuilder.h"
#include "Render/Editor/ImageProcess/IImageProcessStepFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessPipeline.h"
#include "Render/ImageProcess/ImageProcessDefineTarget.h"
#include "Render/ImageProcess/ImageProcessDefineTexture.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Render/ImageProcess/ImageProcessStepBlur.h"
#include "Render/ImageProcess/ImageProcessStepBokeh.h"
#include "Render/ImageProcess/ImageProcessStepChain.h"
#include "Render/ImageProcess/ImageProcessStepDiscardTarget.h"
#include "Render/ImageProcess/ImageProcessStepGodRay.h"
#include "Render/ImageProcess/ImageProcessStepGrain.h"
#include "Render/ImageProcess/ImageProcessStepLensDirt.h"
#include "Render/ImageProcess/ImageProcessStepLuminance.h"
#include "Render/ImageProcess/ImageProcessStepRepeat.h"
#include "Render/ImageProcess/ImageProcessStepSimple.h"
#include "Render/ImageProcess/ImageProcessStepSsao.h"
#include "Render/ImageProcess/ImageProcessStepSmProj.h"
#include "Render/ImageProcess/ImageProcessStepTemporal.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

void gatherSources(const RefArray< ImageProcessStep >& steps, std::vector< std::wstring >& outSources);

Ref< IImageProcessStepFacade > createFacade(const ImageProcessStep* step)
{
	const TypeInfo* facadeType = TypeInfo::find(std::wstring(type_name(step)) + L"Facade");
	if (facadeType)
		return checked_type_cast< IImageProcessStepFacade* >(facadeType->createInstance());
	else
		return 0;
}

void gatherSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources)
{
	Ref< IImageProcessStepFacade > facade = createFacade(step);
	if (facade)
	{
		facade->getSources(step, outSources);

		RefArray< ImageProcessStep > children;
		facade->getChildren(step, children);
		gatherSources(children, outSources);
	}
}

void gatherSources(const RefArray< ImageProcessStep >& steps, std::vector< std::wstring >& outSources)
{
	for (RefArray< ImageProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
		gatherSources(*i, outSources);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessPipeline", 2, ImageProcessPipeline, editor::IPipeline)

bool ImageProcessPipeline::create(const editor::IPipelineSettings* settings)
{
	return true;
}

void ImageProcessPipeline::destroy()
{
}

TypeInfoSet ImageProcessPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ImageProcessSettings >());
	return typeSet;
}

bool ImageProcessPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	const ImageProcessSettings* postProcessSettings = checked_type_cast< const ImageProcessSettings* >(sourceAsset);
	T_FATAL_ASSERT (postProcessSettings);

	const RefArray< ImageProcessDefine >& definitions = postProcessSettings->getDefinitions();
	for (RefArray< ImageProcessDefine >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
	{
		if (const ImageProcessDefineTexture* defineTexture = dynamic_type_cast< const ImageProcessDefineTexture* >(*i))
			pipelineDepends->addDependency(defineTexture->getTexture(), editor::PdfBuild | editor::PdfResource);
	}
	
	RefArray< ImageProcessStep > ss;

	const RefArray< ImageProcessStep >& steps = postProcessSettings->getSteps();
	for (RefArray< ImageProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
		ss.push_back(*i);

	while (!ss.empty())
	{
		Ref< ImageProcessStep > step = ss.front(); ss.pop_front();

		if (const ImageProcessStepBlur* stepBlur = dynamic_type_cast< const ImageProcessStepBlur* >(step))
			pipelineDepends->addDependency(stepBlur->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const ImageProcessStepBokeh* stepBokeh = dynamic_type_cast< const ImageProcessStepBokeh* >(step))
			pipelineDepends->addDependency(stepBokeh->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const ImageProcessStepChain* stepChain = dynamic_type_cast< const ImageProcessStepChain* >(step))
		{
			const RefArray< ImageProcessStep >& steps = stepChain->getSteps();
			for (RefArray< ImageProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
				ss.push_back(*i);
		}
		else if (const ImageProcessStepGodRay* stepGodRay = dynamic_type_cast< const ImageProcessStepGodRay* >(step))
			pipelineDepends->addDependency(stepGodRay->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const ImageProcessStepGrain* stepGrain = dynamic_type_cast< const ImageProcessStepGrain* >(step))
			pipelineDepends->addDependency(stepGrain->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const ImageProcessStepLensDirt* stepLensDirt = dynamic_type_cast< const ImageProcessStepLensDirt* >(step))
			pipelineDepends->addDependency(stepLensDirt->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const ImageProcessStepLuminance* stepLuminance = dynamic_type_cast< const ImageProcessStepLuminance* >(step))
			pipelineDepends->addDependency(stepLuminance->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const ImageProcessStepRepeat* stepRepeat = dynamic_type_cast< const ImageProcessStepRepeat* >(step))
			ss.push_back(stepRepeat->getStep());
		else if (const ImageProcessStepSimple* stepSimple = dynamic_type_cast< const ImageProcessStepSimple* >(step))
			pipelineDepends->addDependency(stepSimple->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const ImageProcessStepSsao* stepSsao = dynamic_type_cast< const ImageProcessStepSsao* >(step))
			pipelineDepends->addDependency(stepSsao->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const ImageProcessStepSmProj* stepSmProj = dynamic_type_cast< const ImageProcessStepSmProj* >(step))
			pipelineDepends->addDependency(stepSmProj->getShader(), editor::PdfBuild | editor::PdfResource);
		else if (const ImageProcessStepTemporal* stepTemporal = dynamic_type_cast< const ImageProcessStepTemporal* >(step))
			pipelineDepends->addDependency(stepTemporal->getShader(), editor::PdfBuild | editor::PdfResource);
	}

	return true;
}

bool ImageProcessPipeline::buildOutput(
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
	Ref< ImageProcessSettings > pp = DeepClone(sourceAsset).create< ImageProcessSettings >();
	if (!pp)
		return false;

	std::set< std::wstring > targets;

	// Get all user defined, non-persistent, targets.
	const RefArray< ImageProcessDefine >& definitions = pp->getDefinitions();
	for (RefArray< ImageProcessDefine >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
	{
		if (const ImageProcessDefineTarget* defineTarget = dynamic_type_cast< const ImageProcessDefineTarget* >(*i))
		{
			if (!defineTarget->persistent())
				targets.insert(defineTarget->getId());
		}
	}

	// Copy steps, insert discard target steps when a user defined target is no longer used
	// to improve target pooling.
	RefArray< ImageProcessStep > outputSteps;
	const RefArray< ImageProcessStep >& steps = pp->getSteps();
	for (RefArray< ImageProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
	{
		Ref< IImageProcessStepFacade > facade = createFacade(*i);
		if (!facade)
		{
			log::error << L"No facade found for post process step " << type_name(*i) << Endl;
			return false;
		}

		outputSteps.push_back(*i);

		// Get sources which are still to be used.
		std::vector< std::wstring > toBeUsedSources;
		for (RefArray< ImageProcessStep >::const_iterator j = i + 1; j != steps.end(); ++j)
			gatherSources(*j, toBeUsedSources);

		// Discard targets which will no longer be used.
		for (std::set< std::wstring >::iterator j = targets.begin(); j != targets.end(); )
		{
			if (std::find(toBeUsedSources.begin(), toBeUsedSources.end(), *j) == toBeUsedSources.end())
			{
				log::info << L"Inserting discard target step \"" << *j << L"\"" << Endl;
				outputSteps.push_back(new ImageProcessStepDiscardTarget(*j));
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

Ref< ISerializable > ImageProcessPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
