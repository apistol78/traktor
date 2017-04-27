/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_IComponentData_H
#define traktor_spark_IComponentData_H

#include "Core/Serialization/ISerializable.h"

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

class Context;
class Sprite;
class IComponent;

/*! \brief Character component interface.
 * \ingroup Spark
 */
class T_DLLCLASS IComponentData : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< IComponent > createInstance(const Context* context, Sprite* owner) const = 0;
};

	}
}

#endif	// traktor_spark_IComponentData_H
