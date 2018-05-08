/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_IInputSource_H
#define traktor_input_IInputSource_H

#include <string>
#include "Core/Object.h"

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

class InputSystem;
class InputValueSet;

/*! \brief Abstract input source.
 * \ingroup Input
 */
class T_DLLCLASS IInputSource : public Object
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const = 0;
	
	virtual void prepare(float T, float dT) = 0;

	/*! \brief Read value from input source.
	 *
	 * \note
	 * Should return NaN if unable to read value
	 * from source as it's possibly used by input
	 * graph to mask features.
	 *
	 * \param T Simulation time.
	 * \param dT Delta time since last read.
	 * \return Input source value.
	 */
	virtual float read(float T, float dT) = 0;
};

	}
}

#endif	// traktor_input_IInputSource_H
