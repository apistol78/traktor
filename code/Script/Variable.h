#ifndef traktor_script_Variable_H
#define traktor_script_Variable_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class IValue;

/*! \brief
 * \ingroup Script
 */
class T_DLLCLASS Variable : public ISerializable
{
	T_RTTI_CLASS;

public:
	Variable();

	Variable(const std::wstring& name, const std::wstring& typeName, const IValue* value);

	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setTypeName(const std::wstring& typeName);

	const std::wstring& getTypeName() const;

	void setValue(const IValue* value);

	const IValue* getValue() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_name;
	std::wstring m_typeName;
	Ref< const IValue > m_value;
};

	}
}

#endif	// traktor_script_Variable_H
