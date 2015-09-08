#ifndef traktor_spark_Frame_H
#define traktor_spark_Frame_H

#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class Frame : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);
};

	}
}

#endif	// traktor_spark_Frame_H
