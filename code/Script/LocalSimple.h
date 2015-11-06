#ifndef traktor_script_LocalSimple_H
#define traktor_script_LocalSimple_H

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
class T_DLLCLASS LocalSimple : public Local
{
	T_RTTI_CLASS;

public:
	LocalSimple();

	LocalSimple(const std::wstring& name, const std::wstring& value);

	const std::wstring& getValue() const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	std::wstring m_value;
};

	}
}

#endif	// traktor_script_LocalSimple_H
