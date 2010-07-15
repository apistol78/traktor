#ifndef traktor_input_InputStateData_H
#define traktor_input_InputStateData_H

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

class IInputNode;

class T_DLLCLASS InputStateData : public ISerializable
{
	T_RTTI_CLASS;
	
public:
	const std::wstring& getId() const;

	const IInputNode* getSource() const;

	virtual bool serialize(ISerializer& s);
	
private:
	std::wstring m_id;
	Ref< IInputNode > m_source;
};
	
	}
}

#endif	// traktor_input_InputStateData_H
