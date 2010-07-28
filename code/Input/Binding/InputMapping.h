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
 * possible to have the source data stored separatly
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
		const InputMappingSourceData* sourceData,
		const InputMappingStateData* stateData
	);

	void update(InputSystem* inputSystem, float dT);

	void reset();

	void setValue(const std::wstring& id, float value);

	float getValue(const std::wstring& id) const;

	IInputSource* getSource(const std::wstring& id) const;

	const std::map< std::wstring, Ref< IInputSource > >& getSources() const;
	
	InputState* getState(const std::wstring& id) const;
	
	const std::map< std::wstring, Ref< InputState > >& getStates() const;
	
private:
	std::map< std::wstring, Ref< IInputSource > > m_sources;
	std::map< std::wstring, Ref< InputState > > m_states;
	InputValueSet m_valueSet;
	float m_T;
};
	
	}
}

#endif	// traktor_input_InputMapping_H
