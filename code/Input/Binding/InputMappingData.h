#ifndef traktor_input_InputMappingData_H
#define traktor_input_InputMappingData_H

#include <map>
#include "Core/Serialization/ISerializable.h"

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
	
class IInputSourceData;
class InputStateData;

class T_DLLCLASS InputMappingData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setSourceData(const std::wstring& id, IInputSourceData* data);
	
	const std::map< std::wstring, Ref< IInputSourceData > >& getSourceData() const;
	
	void setStateData(const std::wstring& id, InputStateData* data);
	
	const std::map< std::wstring, Ref< InputStateData > >& getStateData() const;

	virtual bool serialize(ISerializer& s);

private:
	std::map< std::wstring, Ref< IInputSourceData > > m_sourceData;
	std::map< std::wstring, Ref< InputStateData > > m_stateData;
};

	}
}

#endif	// traktor_input_InputMappingData_H
