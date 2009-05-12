#ifndef traktor_i18n_Text_H
#define traktor_i18n_Text_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace i18n
	{

/*! \brief Automatic text localization.
 *
 * Used as an implicit bridge from ID to localized text.
 */
class T_DLLCLASS Text : public Object
{
	T_RTTI_CLASS(Text)

public:
	Text(const std::wstring& id, const std::wstring& defaultText = L"");

	const std::wstring& str() const;

	operator const std::wstring& () const;

private:
	std::wstring m_text;
};

	}
}

#endif	// traktor_i18n_Text_H
