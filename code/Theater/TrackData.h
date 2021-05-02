#pragma once

#include "Core/Guid.h"
#include "Core/Math/TransformPath.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace theater
	{

/*! Track data.
 * \ingroup Theater
 */
class T_DLLCLASS TrackData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setEntityId(const Guid& entityId);

	const Guid& getEntityId() const;

	void setLookAtEntityId(const Guid& entityId);

	const Guid& getLookAtEntityId() const;

	void setPath(const TransformPath& path);

	const TransformPath& getPath() const;

	TransformPath& getPath();

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_entityId;
	Guid m_lookAtEntityId;
	TransformPath m_path;
};

	}
}

