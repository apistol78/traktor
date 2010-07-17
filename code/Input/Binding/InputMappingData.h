#ifndef traktor_input_InputMappingData_H
#define traktor_input_InputMappingData_H

#include "Core/RefArray.h"
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
	
class InputStateData;
class InputValueSourceData;

class T_DLLCLASS InputMappingData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addSourceData(InputValueSourceData* data);
	
	const RefArray< InputValueSourceData >& getSourceData() const;
	
	void addStateData(InputStateData* data);
	
	const RefArray< InputStateData >& getStateData() const;

	virtual bool serialize(ISerializer& s);

private:
	RefArray< InputValueSourceData > m_sourceData;
	RefArray< InputStateData > m_stateData;
};

	}
}

#endif	// traktor_input_InputMappingData_H
