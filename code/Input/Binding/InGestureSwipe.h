#ifndef traktor_input_InGestureSwipe_H
#define traktor_input_InGestureSwipe_H

#include "Input/Binding/IInputNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InGestureSwipe : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	enum SwipeDirection
	{
		SdUp,
		SdDown,
		SdLeft,
		SdRight
	};

	InGestureSwipe();

	InGestureSwipe(
		IInputNode* sourceActive,
		IInputNode* sourceX,
		IInputNode* sourceY,
		SwipeDirection direction
	);
	
	virtual Ref< Instance > createInstance() const;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const;

	virtual void serialize(ISerializer& s);
	
private:
	Ref< IInputNode > m_sourceActive;
	Ref< IInputNode > m_sourceX;
	Ref< IInputNode > m_sourceY;
	SwipeDirection m_direction;
};

	}
}

#endif	// traktor_input_InGestureSwipe_H
