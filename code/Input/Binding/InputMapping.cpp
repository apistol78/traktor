#include "Input/Binding/InputMapping.h"
#include "Input/Binding/InputMappingData.h"
#include "Input/Binding/InputState.h"
#include "Input/Binding/InputStateData.h"
#include "Input/Binding/InputValueSource.h"

namespace traktor
{
	namespace input
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputMapping", InputMapping, Object)

bool InputMapping::create(const InputMappingData* data)
{
	const RefArray< InputValueSourceData >& sourceData = data->getSourceData();
	for (RefArray< InputValueSourceData >::const_iterator i = sourceData.begin(); i != sourceData.end(); ++i)
		m_sources.push_back(new InputValueSource(*i));
		
	const RefArray< InputStateData >& stateData = data->getStateData();
	for (RefArray< InputStateData >::const_iterator i = stateData.begin(); i != stateData.end(); ++i)
	{
		m_states.insert(std::make_pair(
			(*i)->getId(),
			new InputState(*i)
		));
	}
	
	return true;
}

void InputMapping::update(InputSystem* InputSystem)
{
	// Input value set by updating all sources.
	for (RefArray< InputValueSource >::iterator i = m_sources.begin(); i != m_sources.end(); ++i)
		(*i)->update(InputSystem, m_valueSet);
		
	// Update states.
	for (std::map< std::wstring, Ref< InputState > >::iterator i = m_states.begin(); i != m_states.end(); ++i)
		i->second->update(m_valueSet);
}

InputState* InputMapping::get(const std::wstring& id) const
{
	std::map< std::wstring, Ref< InputState > >::const_iterator i = m_states.find(id);
	return i != m_states.end() ? i->second : 0;
}
	
	}
}
