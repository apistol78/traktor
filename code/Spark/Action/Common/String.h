#pragma once

#include <string>
#include "Spark/Action/ActionObjectRelay.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! ActionScript string wrapper.
 * \ingroup Spark
 */
class T_DLLCLASS String : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	String();

	explicit String(char ch);

	explicit String(const std::string& str);

	void set(const std::string& str) { m_str = str; }

	const std::string& get() const { return m_str; }

private:
	std::string m_str;
};

	}
}

