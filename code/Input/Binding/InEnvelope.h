/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InEnvelope_H
#define traktor_input_InEnvelope_H

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
class T_DLLCLASS InEnvelope : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	InEnvelope();

	InEnvelope(IInputNode* source, float delay);
	
	virtual Ref< Instance > createInstance() const T_OVERRIDE T_FINAL;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
	
private:
	friend class InEnvelopeTraits;

	Ref< IInputNode > m_source;
	float m_keys[4];
	float m_center;
	float m_width;
};

	}
}

#endif	// traktor_input_InEnvelope_H
