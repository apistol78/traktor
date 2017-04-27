/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InputMapping_H
#define traktor_input_InputMapping_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Input/InputTypes.h"
#include "Input/Binding/InputValueSet.h"

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

class DeviceControlManager;
class IInputSource;
class InputMappingSourceData;
class InputMappingStateData;
class InputState;
class InputSystem;

/*! \brief Input sources to state mapping.
 * \ingroup Input
 *
 * Each source is mapped into a linear array
 * of values (each value tagged with an unique, literal, id)
 * and are then consumed by defined states.
 *
 * Each state may have a chain of input nodes in order
 * to modify the source values into the final state's value.
 *
 * As source and state data are separated it's
 * possible to have the source data stored separately
 * from the state data and thus have sources be
 * reconfigurable by the user and stored
 * in a configuration file of some sort.
 */
class T_DLLCLASS InputMapping : public Object
{
	T_RTTI_CLASS;
	
public:
	InputMapping();
	
	bool create(
		InputSystem* inputSystem,
		const InputMappingSourceData* sourceData,
		const InputMappingStateData* stateData
	);

	void update(float dT, bool inputEnable);

	void reset();

	void reset(handle_t id);

	void setValue(handle_t id, float value);

	float getValue(handle_t id) const;

	IInputSource* getSource(handle_t id) const;

	const SmallMap< handle_t, Ref< IInputSource > >& getSources() const;
	
	InputState* getState(handle_t id) const;
	
	const SmallMap< handle_t, Ref< InputState > >& getStates() const;

	/*! \name Helpers */
	// \{

	float getStateValue(handle_t id) const;

	float getStatePreviousValue(handle_t id) const;

	float getStateDeltaValue(handle_t id) const;

	bool isStateDown(handle_t id) const;

	bool isStateUp(handle_t id) const;

	bool isStatePressed(handle_t id) const;

	bool isStateReleased(handle_t id) const;

	bool hasStateChanged(handle_t id) const;

	// \}

	float getIdleDuration() const;

	/*! \name Access by string handle. */
	// \{

	void reset(const std::wstring& id) { reset(getParameterHandle(id)); }

	void setValue(const std::wstring& id, float value) { setValue(getParameterHandle(id), value); }

	float getValue(const std::wstring& id) const { return getValue(getParameterHandle(id)); }

	IInputSource* getSource(const std::wstring& id) { return getSource(getParameterHandle(id)); }

	InputState* getState(const std::wstring& id) const { return getState(getParameterHandle(id)); }

	float getStateValue(const std::wstring& id) const { return getStateValue(getParameterHandle(id)); }

	float getStatePreviousValue(const std::wstring& id) const { return getStatePreviousValue(getParameterHandle(id)); }

	float getStateDeltaValue(const std::wstring& id) const { return getStateDeltaValue(getParameterHandle(id)); }

	bool isStateDown(const std::wstring& id) const { return isStateDown(getParameterHandle(id)); }

	bool isStateUp(const std::wstring& id) const { return isStateUp(getParameterHandle(id)); }

	bool isStatePressed(const std::wstring& id) const { return isStatePressed(getParameterHandle(id)); }

	bool isStateReleased(const std::wstring& id) const { return isStateReleased(getParameterHandle(id)); }

	bool hasStateChanged(const std::wstring& id) const { return hasStateChanged(getParameterHandle(id)); }

	// \}
	
private:
	Ref< DeviceControlManager > m_deviceControlManager;
	SmallMap< handle_t, Ref< IInputSource > > m_sources;
	SmallMap< handle_t, Ref< InputState > > m_states;
	InputValueSet m_valueSet;
	float m_idleTimer;
	float m_T;
};
	
	}
}

#endif	// traktor_input_InputMapping_H
