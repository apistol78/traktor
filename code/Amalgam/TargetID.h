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

/*! \brief Running target identification.
 * \ingroup Amalgam
 */
class T_DLLCLASS TargetID : public ISerializable
{
	T_RTTI_CLASS;

public:
	TargetID();

	TargetID(const Guid& id, const std::wstring& name);

	const Guid& getId() const;

	const std::wstring& getName() const;

	virtual void serialize(ISerializer& s);

private:
	Guid m_id;
	std::wstring m_name;
};

	}
}

#endif	// traktor_amalgam_TargetID_H
