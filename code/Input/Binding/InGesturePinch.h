#ifndef traktor_input_InGesturePinch_H
#define traktor_input_InGesturePinch_H

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
class T_DLLCLASS InGesturePinch : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	enum PinchDirection
	{
		PdAny,
		PdX,
		PdY
	};

	InGesturePinch();

	InGesturePinch(
		IInputNode* sourceActive,
		IInputNode* sourceX1,
		IInputNode* sourceY1,
		IInputNode* sourceX2,
		IInputNode* sourceY2
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
	friend class InGesturePinchTraits;

	Ref< IInputNode > m_sourceActive;
	Ref< IInputNode > m_sourceX1;
	Ref< IInputNode > m_sourceY1;
	Ref< IInputNode > m_sourceX2;
	Ref< IInputNode > m_sourceY2;
	PinchDirection m_direction;
};

	}
}

#endif	// traktor_input_InGesturePinch_H
