#ifndef traktor_amalgam_TargetID_H
#define traktor_amalgam_TargetID_H

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class T_DLLCLASS TargetID : public ISerializable
{
	T_RTTI_CLASS;

public:
	TargetID();

	TargetID(const Guid& id);

	const Guid& getId() const;

	virtual void serialize(ISerializer& s);

private:
	Guid m_id;
};

	}
}

#endif	// traktor_amalgam_TargetID_H
