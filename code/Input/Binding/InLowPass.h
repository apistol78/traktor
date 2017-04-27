/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InLowPass_H
#define traktor_input_InLowPass_H

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
class T_DLLCLASS InLowPass : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	InLowPass();

	InLowPass(IInputNode* source, IInputNode* coeff);
	
	virtual Ref< Instance > createInstance() const T_OVERRIDE T_FINAL;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
	
private:
	friend class InLowPassTraits;

	Ref< IInputNode > m_source;
	Ref< IInputNode > m_coeff;
};

	}
}

#endif	// traktor_input_InLowPass_H
