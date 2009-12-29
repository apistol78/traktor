#ifndef traktor_flash_ActionString_H
#define traktor_flash_ActionString_H

#include "Flash/Action/ActionObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief ActionScript string wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS ActionString : public ActionObject
{
	T_RTTI_CLASS;

public:
	ActionString();

	explicit ActionString(wchar_t ch);

	explicit ActionString(const std::wstring& str);

	const std::wstring& get() const { return m_str; }

	virtual std::wstring toString() const;

private:
	std::wstring m_str;
};

	}
}

#endif	// traktor_flash_ActionString_H
