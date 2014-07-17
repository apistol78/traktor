#include "Core/Log/Log.h"
#include "Core/Math/Vector2.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/InGestureTap.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const float c_deviateThreshold = 2.0f;

struct InGestureTapInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceActiveInstance;
	Ref< IInputNode::Instance > sourceXInstance;
	Ref< IInputNode::Instance > sourceYInstance;
	Ref< IInputNode::Instance > fixedXInstance;
	Ref< IInputNode::Instance > fixedYInstance;
	Vector2 first;
	bool haveFirst;
	bool valid;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InGestureTap", 0, InGestureTap, IInputNode)

InGestureTap::InGestureTap()
{
}

Ref< IInputNode::Instance > InGestureTap::createInstance() const
{
	Ref< InGestureTapInstance > instance = new InGestureTapInstance();
	instance->sourceActiveInstance = m_sourceActive->createInstance();
	instance->sourceXInstance = m_sourceX->createInstance();
	instance->sourceYInstance = m_sourceY->createInstance();

	if (m_fixedX)
		instance->fixedXInstance = m_fixedX->createInstance();
	if (m_fixedY)
		instance->fixedYInstance = m_fixedY->createInstance();

	instance->haveFirst = false;
	instance->valid = false;
	return instance;
}

float InGestureTap::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InGestureTapInstance* swipeInstance = static_cast< InGestureTapInstance* >(instance);

	float V = m_sourceActive->evaluate(swipeInstance->sourceActiveInstance, valueSet, T, dT);
	if (!asBoolean(V))
	{
		bool result = swipeInstance->valid;
		
		swipeInstance->valid = false;
		swipeInstance->haveFirst = false;

		return asFloat(result);
	}

	// Swipe is active; sample positions.
	float X = m_sourceX->evaluate(swipeInstance->sourceXInstance, valueSet, T, dT);
	float Y = m_sourceY->evaluate(swipeInstance->sourceYInstance, valueSet, T, dT);

	Vector2 p(X, Y);

	if (!swipeInstance->haveFirst)
	{
		swipeInstance->first = p;

		if (swipeInstance->fixedXInstance)
			swipeInstance->first.x = m_fixedX->evaluate(swipeInstance->fixedXInstance, valueSet, T, dT);

		if (swipeInstance->fixedYInstance)
			swipeInstance->first.y = m_fixedY->evaluate(swipeInstance->fixedYInstance, valueSet, T, dT);

		swipeInstance->valid = true;
		swipeInstance->haveFirst = true;
	}

	swipeInstance->valid &= bool((swipeInstance->first - p).length() < c_deviateThreshold);

	return asFloat(false);
}

void InGestureTap::serialize(ISerializer& s)
{
	s >> MemberRef< IInputNode >(L"sourceActive", m_sourceActive);
	s >> MemberRef< IInputNode >(L"sourceX", m_sourceX);
	s >> MemberRef< IInputNode >(L"sourceY", m_sourceY);
	s >> MemberRef< IInputNode >(L"fixedX", m_fixedX);
	s >> MemberRef< IInputNode >(L"fixedY", m_fixedY);
}
	
	}
}
