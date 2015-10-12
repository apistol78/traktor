#ifndef traktor_Environment_H
#define traktor_Environment_H

#include <map>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief OS environment.
 * \ingroup Core
 */
class T_DLLCLASS Environment : public Object
{
	T_RTTI_CLASS;

public:
	void set(const std::wstring& key, const std::wstring& value);

	void insert(const std::map< std::wstring, std::wstring >& env);

	bool has(const std::wstring& key) const;

	std::wstring get(const std::wstring& key) const;

	const std::map< std::wstring, std::wstring >& get() const;

private:
	std::map< std::wstring, std::wstring > m_env;
};

}

#endif	// traktor_Environment_H
