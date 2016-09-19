#ifndef traktor_input_InputMappingStateData_H
#define traktor_input_InputMappingStateData_H

#include <map>
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

/*! \brief Input mapping state data
 * \ingroup Input
 *
 * Serializable description of input states.
 */
class T_DLLCLASS InputMappingStateData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setStateData(const std::wstring& id, InputStateData* data);
	
	const std::map< std::wstring, Ref< InputStateData > >& getStateData() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::map< std::wstring, Ref< InputStateData > > m_stateData;
};

	}
}

#endif	// traktor_input_InputMappingStateData_H
