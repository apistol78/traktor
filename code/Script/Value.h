#ifndef traktor_script_Value_H
#define traktor_script_Value_H

#include "Script/IValue.h"

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

/*! \brief
 * \ingroup Script
 */
class T_DLLCLASS Value : public IValue
{
	T_RTTI_CLASS;

public:
	Value();

	Value(const std::wstring& literal);

	void setLiteral(const std::wstring& literal);

	const std::wstring& getLiteral() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_literal;
};

	}
}

#endif	// traktor_script_Value_H
