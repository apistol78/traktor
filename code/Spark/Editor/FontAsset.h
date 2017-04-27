/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_FontAsset_H
#define traktor_spark_FontAsset_H

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS FontAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class FontPipeline;

	std::wstring m_includeCharacters;
};

	}
}

#endif	// traktor_spark_FontAsset_H
