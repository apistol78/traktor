/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_ExternalData_H
#define traktor_spark_ExternalData_H

#include "Resource/Id.h"
#include "Spark/CharacterData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! \brief ExternalData reference to character.
 * \ingroup Spark
 */
class T_DLLCLASS ExternalData : public CharacterData
{
	T_RTTI_CLASS;

public:
	ExternalData();

	ExternalData(const resource::Id< CharacterData >& reference);

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class CharacterPipeline;
	friend class ExternalFactory;

	resource::Id< CharacterData > m_reference;
};

	}
}

#endif	// traktor_spark_ExternalData_H
