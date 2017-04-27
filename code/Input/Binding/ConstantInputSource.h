/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_ConstantInputSource_H
#define traktor_input_ConstantInputSource_H

#include <list>
#include "Input/Binding/IInputSource.h"

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

/*! \brief Constant value input.
 * \ingroup Input
 */
class T_DLLCLASS ConstantInputSource : public IInputSource
{
	T_RTTI_CLASS;

public:
	ConstantInputSource(float value);
	
	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual void prepare(float T, float dT) T_OVERRIDE T_FINAL;

	virtual float read(float T, float dT) T_OVERRIDE T_FINAL;
	
private:
	float m_value;
};

	}
}

#endif	// traktor_input_ConstantInputSource_H
