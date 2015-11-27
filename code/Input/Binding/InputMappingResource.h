#ifndef traktor_input_InputMappingResource_H
#define traktor_input_InputMappingResource_H

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

class InputMappingSourceData;
class InputMappingStateData;

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InputMappingResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	InputMappingResource();

	InputMappingResource(InputMappingSourceData* sourceData, InputMappingStateData* stateData);

	InputMappingSourceData* getSourceData() const { return m_sourceData; }

	InputMappingStateData* getStateData() const { return m_stateData; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	Ref< InputMappingSourceData > m_sourceData;
	Ref< InputMappingStateData > m_stateData;
};

	}
}

#endif	// traktor_input_InputMappingResource_H
