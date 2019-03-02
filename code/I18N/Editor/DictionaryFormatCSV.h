#pragma once

#include "I18N/Editor/IDictionaryFormat.h"

namespace traktor
{
	namespace i18n
	{

class DictionaryFormatCSV : public IDictionaryFormat
{
	T_RTTI_CLASS;

public:
	virtual bool supportExtension(const std::wstring& extension) const override final;

	virtual Ref< Dictionary > read(IStream* stream, int32_t keyColumn, int32_t textColumn) const override final;

	virtual bool write(IStream* stream, const Dictionary* dictionary) const override final;
};

	}
}

