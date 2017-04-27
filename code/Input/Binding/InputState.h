/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InputState_H
#define traktor_input_InputState_H

#include "Core/Object.h"
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
	
class InputStateData;
class InputValueSet;

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InputState : public Object
{
	T_RTTI_CLASS;

public:
	InputState();
	
	bool create(const InputStateData* data);

	void update(const InputValueSet& valueSet, float T, float dT);

	void reset();
	
	float getValue() const { return m_currentValue; }
	
	float getPreviousValue() const { return m_previousValue; }
	
	bool isDown() const;
	
	bool isUp() const;
	
	bool isPressed() const;
	
	bool isReleased() const;

	bool hasChanged() const;
	
private:
	Ref< const InputStateData > m_data;
	Ref< IInputNode::Instance > m_instance;
	bool m_active;
	float m_previousValue;
	float m_currentValue;
};

	}
}

#endif	// traktor_input_InputState_H
