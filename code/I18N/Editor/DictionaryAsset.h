/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_i18n_AnimationAsset_H
#define traktor_i18n_AnimationAsset_H

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace i18n
	{

class T_DLLCLASS DictionaryAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	DictionaryAsset();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	int32_t getKeyColumn() const { return m_keyColumn; }

	int32_t getTextColumn() const { return m_textColumn; }

private:
	int32_t m_keyColumn;
	int32_t m_textColumn;
};

	}
}

#endif	// traktor_i18n_AnimationAsset_H
