/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "I18N/Editor/DictionaryAsset.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.i18n.DictionaryAsset", 0, DictionaryAsset, editor::Asset)

DictionaryAsset::DictionaryAsset()
:	m_keyColumn(0)
,	m_textColumn(1)
{
}

void DictionaryAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	s >> Member< int32_t >(L"keyColumn", m_keyColumn);
	s >> Member< int32_t >(L"textColumn", m_textColumn);
}

	}
}
