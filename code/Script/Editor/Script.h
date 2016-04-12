#ifndef traktor_script_Script_H
#define traktor_script_Script_H

#include <functional>
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

/*! \brief Persistent script.
 * \ingroup Script
 */
class T_DLLCLASS Script : public ISerializable
{
	T_RTTI_CLASS;

public:
	Script();

	Script(const std::wstring& text);

	void setTextDirect(const std::wstring& text);

	std::wstring escape(std::function< std::wstring (const Guid& g) > fn) const;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_text;
};

	}
}

#endif	// traktor_script_Script_H
