#ifndef traktor_script_Local_H
#define traktor_script_Local_H

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

/*! \brief
 * \ingroup Script
 */
class T_DLLCLASS Local : public ISerializable
{
	T_RTTI_CLASS;

public:
	Local();

	Local(const std::wstring& name);

	const std::wstring& getName() const;

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_name;
};

	}
}

#endif	// traktor_script_Local_H
