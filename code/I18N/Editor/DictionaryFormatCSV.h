/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_i18n_DictionaryFormatCSV_H
#define traktor_i18n_DictionaryFormatCSV_H

#include "I18N/Editor/IDictionaryFormat.h"

namespace traktor
{
	namespace i18n
	{

class DictionaryFormatCSV : public IDictionaryFormat
{
	T_RTTI_CLASS;

public:
	virtual bool supportExtension(const std::wstring& extension) const T_OVERRIDE T_FINAL;

	virtual Ref< Dictionary > read(IStream* stream, int32_t keyColumn, int32_t textColumn) const T_OVERRIDE T_FINAL;

	virtual bool write(IStream* stream, const Dictionary* dictionary) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_i18n_DictionaryFormatCSV_H
