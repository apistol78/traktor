#include "Animation/Editor/StatePipeline.h"
#include "Animation/Animation/State.h"
#include "Animation/Animation/StateGraph.h"
#include "Animation/Animation/StatePoseControllerData.h"
#include "Editor/IPipelineManager.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.animation.StatePipeline", StatePipeline, editor::DefaultPipeline)

TypeSet StatePipeline::getAssetTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< State >());
	typeSet.insert(&type_of< StateGraph >());
	typeSet.insert(&type_of< StatePoseControllerData >());
	return typeSet;
}

bool StatePipeline::buildDependencies(
	editor::IPipelineManager* pipelineManager,
	const db::Instance* sourceInstance,
	const Serializable* sourceAsset,
	Ref< const Object >& outBuildParams
) const
{
	if (const State* state = dynamic_type_cast< const State* >(sourceAsset))
		pipelineManager->addDependency(state->getAnimation().getGuid(), true);
	else if (const StateGraph* stateGraph = dynamic_type_cast< const StateGraph* >(sourceAsset))
	{
		const RefArray< State >& states = stateGraph->getStates();
		for (RefArray< State >::const_iterator i = states.begin(); i != states.end(); ++i)
			pipelineManager->addDependency(*i);
	}
	else if (const StatePoseControllerData* controllerData = dynamic_type_cast< const StatePoseControllerData* >(sourceAsset))
		pipelineManager->addDependency(controllerData->getStateGraph().getGuid(), true);

	return true;
}

	}
}
