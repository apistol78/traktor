#include "Core/Containers/CircularVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Vector2.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberRef.h"
#include "Input/Binding/InGestureSwipe.h"
#include "Input/Binding/InputValueSet.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const float c_sampleThreshold = 2.0f;
const float c_deviateThreshold = 20.0f;

struct InGestureSwipeInstance : public RefCountImpl< IInputNode::Instance >
{
	Ref< IInputNode::Instance > sourceActiveInstance;
	Ref< IInputNode::Instance > sourceXInstance;
	Ref< IInputNode::Instance > sourceYInstance;
	CircularVector< Vector2, 128 > points;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InGestureSwipe", 0, InGestureSwipe, IInputNode)

InGestureSwipe::InGestureSwipe()
{
}

InGestureSwipe::InGestureSwipe(
	IInputNode* sourceActive,
	IInputNode* sourceX,
	IInputNode* sourceY,
	SwipeDirection direction
)
:	m_sourceActive(sourceActive)
,	m_sourceX(sourceX)
,	m_sourceY(sourceY)
,	m_direction(direction)
{
}

Ref< IInputNode::Instance > InGestureSwipe::createInstance() const
{
	Ref< InGestureSwipeInstance > instance = new InGestureSwipeInstance();
	instance->sourceActiveInstance = m_sourceActive->createInstance();
	instance->sourceXInstance = m_sourceX->createInstance();
	instance->sourceYInstance = m_sourceY->createInstance();
	return instance;
}

float InGestureSwipe::evaluate(
	Instance* instance,
	const InputValueSet& valueSet,
	float T,
	float dT
) const
{
	InGestureSwipeInstance* swipeInstance = static_cast< InGestureSwipeInstance* >(instance);

	float V = m_sourceActive->evaluate(swipeInstance->sourceActiveInstance, valueSet, T, dT);
	if (!asBoolean(V))
	{
		bool result = false;

		// If any points has been sampled we need to match with swipe gesture.
		if (!swipeInstance->points.empty())
		{
			// Determine if sampled points match desired swipe gesture.
			const CircularVector< Vector2, 128 >& points = swipeInstance->points;
			if (points.size() >= 3)
			{
				// Determine swipe direction from first and last point.
				Vector2 dir = (points.back() - points.front()).normalized();
				Vector2 adir(abs(dir.x), abs(dir.y));

				switch (m_direction)
				{
				case SdUp:
					result = bool(dir.y < 0.0f && adir.y > adir.x);
					break;

				case SdDown:
					result = bool(dir.y > 0.0f && adir.y > adir.x);
					break;

				case SdLeft:
					result = bool(dir.x < 0.0f && adir.x > adir.y);
					break;

				case SdRight:
					result = bool(dir.x > 0.0f && adir.x > adir.y);
					break;

				default:
					break;
				}

				if (result)
				{
					// Calculate point set average mid point.
					Vector2 mid(0.0f, 0.0f);

					for (uint32_t i = 0; i < points.size(); ++i)
						mid += points[i];

					mid /= float(points.size());

					// Ensure no point deviate too much from direction.
					for (uint32_t i = 0; i < points.size(); ++i)
					{
						float k = abs(dot(dir.perpendicular(), points[i] - mid));
						result &= bool(k <= c_deviateThreshold);
					}
				}
			}

			swipeInstance->points.clear();
		}

		return asFloat(result);
	}

	// Swipe is active; sample positions.
	float X = m_sourceX->evaluate(swipeInstance->sourceXInstance, valueSet, T, dT);
	float Y = m_sourceY->evaluate(swipeInstance->sourceYInstance, valueSet, T, dT);

	if (!swipeInstance->points.empty())
	{
		// Ensure distance from last sample.
		Vector2 delta = Vector2(X, Y) - swipeInstance->points.back();
		if (delta.length() >= c_sampleThreshold)
			swipeInstance->points.push_back(Vector2(X, Y));
	}
	else
		swipeInstance->points.push_back(Vector2(X, Y));

	return asFloat(false);
}

void InGestureSwipe::serialize(ISerializer& s)
{
	const MemberEnum< SwipeDirection >::Key c_SwipeDirection_Keys[] =
	{
		{ L"SdUp", SdUp },
		{ L"SdDown", SdDown },
		{ L"SdLeft", SdLeft },
		{ L"SdRight", SdRight },
		0
	};

	s >> MemberRef< IInputNode >(L"sourceActive", m_sourceActive);
	s >> MemberRef< IInputNode >(L"sourceX", m_sourceX);
	s >> MemberRef< IInputNode >(L"sourceY", m_sourceY);
	s >> MemberEnum< SwipeDirection >(L"direction", m_direction, c_SwipeDirection_Keys);
}
	
	}
}
