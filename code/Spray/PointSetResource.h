#ifndef traktor_spray_PointSetResource_H
#define traktor_spray_PointSetResource_H

#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace spray
	{
	
class T_DLLCLASS PointSetResource : public ISerializable
{
	T_RTTI_CLASS;
	
public:
	virtual bool serialize(ISerializer& s);
};
	
	}
}

#endif	// traktor_spray_PointSetResource_H
