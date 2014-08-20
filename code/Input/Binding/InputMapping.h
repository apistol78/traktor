#ifndef traktor_input_InputMapping_H
#define traktor_input_InputMapping_H

#include "Core/Object.h"
#include "Core/RefArray.h"
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
class IInputFilter;
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

	void reset(const std::wstring& id);

	void setValue(const std::wstring& id, float value);

	float getValue(const std::wstring& id) const;

	IInputSource* getSource(const std::wstring& id) const;

	const std::map< std::wstring, Ref< IInputSource > >& getSources() const;
	
	InputState* getState(const std::wstring& id) const;
	
	const std::map< std::wstring, Ref< InputState > >& getStates() const;

	/*! \name Helpers */
	// \{

	float getStateValue(const std::wstring& id) const;

	float getStatePreviousValue(const std::wstring& id) const;

	float getStateDeltaValue(const std::wstring& id) const;

	bool isStateDown(const std::wstring& id) const;

	bool isStateUp(const std::wstring& id) const;

	bool isStatePressed(const std::wstring& id) const;

	bool isStateReleased(const std::wstring& id) const;

	bool hasStateChanged(const std::wstring& id) const;

	// \}

	float getIdleDuration() const;
	
private:
	Ref< DeviceControlManager > m_deviceControlManager;
	std::map< std::wstring, Ref< IInputSource > > m_sources;
	RefArray< IInputFilter > m_filters;
	std::map< std::wstring, Ref< InputState > > m_states;
	InputValueSet m_valueSet;
	float m_idleTimer;
	float m_T;
};
	
	}
}

#endif	// traktor_input_InputMapping_H
