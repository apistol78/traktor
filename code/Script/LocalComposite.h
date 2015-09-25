#ifndef traktor_script_LocalComposite_H
#define traktor_script_LocalComposite_H

#include "Core/RefArray.h"
#include "Script/Local.h"

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
class T_DLLCLASS LocalComposite : public Local
{
	T_RTTI_CLASS;

public:
	LocalComposite();

	LocalComposite(const std::wstring& name, const std::wstring& value, const RefArray< Local >& values);

	const std::wstring& getValue() const;

	const RefArray< Local >& getValues() const;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_value;
	RefArray< Local > m_values;
};

	}
}

#endif	// traktor_script_LocalComposite_H
