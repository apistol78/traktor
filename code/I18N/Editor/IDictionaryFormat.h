/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_i18n_IDictionaryFormat_H
#define traktor_i18n_IDictionaryFormat_H

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class IStream;
class Path;

	namespace i18n
	{

class Dictionary;

class IDictionaryFormat : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool supportExtension(const std::wstring& extension) const = 0;

	virtual Ref< Dictionary > read(IStream* stream, int32_t keyColumn, int32_t textColumn) const = 0;

	virtual bool write(IStream* stream, const Dictionary* dictionary) const = 0;

	static Ref< Dictionary > readAny(const Path& filePath, int32_t keyColumn, int32_t textColumn);

	static Ref< Dictionary > readAny(IStream* stream, const std::wstring& extension, int32_t keyColumn, int32_t textColumn);

	static bool writeAny(const Path& filePath, const Dictionary* dictionary);

	static bool writeAny(IStream* stream, const std::wstring& extension, const Dictionary* dictionary);
};

	}
}

#endif	// traktor_i18n_IDictionaryFormat_H
