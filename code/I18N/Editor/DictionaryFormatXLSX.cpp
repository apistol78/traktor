#include "Core/Misc/String.h"
#include "I18N/Editor/DictionaryFormatXLSX.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.i18n.DictionaryFormatXLSX", 0, DictionaryFormatXLSX, IDictionaryFormat)

bool DictionaryFormatXLSX::supportExtension(const std::wstring& extension) const
{
	return compareIgnoreCase< std::wstring >(extension, L"xlsx") == 0;
}

Ref< Dictionary > DictionaryFormatXLSX::read(IStream* stream, int32_t keyColumn, int32_t textColumn) const
{
	return 0;
}

bool DictionaryFormatXLSX::write(IStream* stream, const Dictionary* dictionary) const
{
	return false;
}

	}
}
