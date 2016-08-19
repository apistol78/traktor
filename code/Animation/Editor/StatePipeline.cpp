#include "Animation/Animation/SimpleAnimationControllerData.h"
#include "Animation/Animation/StateNodeAnimation.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/StatePoseControllerData.h"
#include "Animation/Editor/StatePipeline.h"
#include "Editor/IPipelineDepends.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.StatePipeline", 0, StatePipeline, editor::DefaultPipeline)

TypeInfoSet StatePipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SimpleAnimationControllerData >());
	typeSet.insert(&type_of< StateNode >());
	typeSet.insert(&type_of< StateNodeAnimation >());
	typeSet.insert(&type_of< StateGraph >());
	typeSet.insert(&type_of< StatePoseControllerData >());
	return typeSet;
}

bool StatePipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const SimpleAnimationControllerData* simpleControllerData = dynamic_type_cast< const SimpleAnimationControllerData* >(sourceAsset))
		pipelineDepends->addDependency(simpleControllerData->getAnimation(), editor::PdfBuild | editor::PdfResource);
	else if (const StateNodeAnimation* state = dynamic_type_cast< const StateNodeAnimation* >(sourceAsset))
		pipelineDepends->addDependency(state->getAnimation().getId(), editor::PdfBuild | editor::PdfResource);
	else if (const StateGraph* stateGraph = dynamic_type_cast< const StateGraph* >(sourceAsset))
	{
		const RefArray< StateNode >& states = stateGraph->getStates();
		for (RefArray< StateNode >::const_iterator i = states.begin(); i != states.end(); ++i)
			pipelineDepends->addDependency(*i);
	}
	else if (const StatePoseControllerData* controllerData = dynamic_type_cast< const StatePoseControllerData* >(sourceAsset))
		pipelineDepends->addDependency(controllerData->getStateGraph(), editor::PdfBuild);

	return true;
}

	}
}
