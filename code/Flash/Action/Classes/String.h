#ifndef traktor_flash_String_H
#define traktor_flash_String_H

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
class T_DLLCLASS String : public ActionObject
{
	T_RTTI_CLASS;

public:
	String();

	explicit String(wchar_t ch);

	explicit String(const std::wstring& str);

	const std::wstring& get() const { return m_str; }

	virtual std::wstring toString() const;

private:
	std::wstring m_str;
};

	}
}

#endif	// traktor_flash_String_H
