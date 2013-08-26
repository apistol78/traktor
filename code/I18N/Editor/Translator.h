#ifndef traktor_i18n_TranslateWizardTool_H
#define traktor_i18n_TranslateWizardTool_H

#include "Core/Object.h"

namespace traktor
{
	namespace i18n
	{

class Translator : public Object
{
	T_RTTI_CLASS;

public:
	Translator(const std::wstring& from, const std::wstring& to);

	bool translate(const std::wstring& text, std::wstring& outText) const;

private:
	std::wstring m_from;
	std::wstring m_to;
};

	}
}

#endif	// traktor_i18n_TranslateWizardTool_H
