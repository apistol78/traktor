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
	TrackData();

	void setEntityId(const Guid& entityId);

	const Guid& getEntityId() const;

	void setLookAtEntityId(const Guid& entityId);

	const Guid& getLookAtEntityId() const;

	void setPath(const TransformPath& path);

	const TransformPath& getPath() const;

	TransformPath& getPath();

	void setLoopStart(float loopStart);

	float getLoopStart() const;

	void setLoopEnd(float loopEnd);

	float getLoopEnd() const;

	void setTimeOffset(float timeOffset);

	float getTimeOffset() const;

	void setWobbleMagnitude(float wobbleMagnitude);

	float getWobbleMagnitude() const;

	void setWobbleRate(float wobbleRate);

	float getWobbleRate() const;

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_entityId;
	Guid m_lookAtEntityId;
	TransformPath m_path;
	float m_loopStart;
	float m_loopEnd;
	float m_timeOffset;
	float m_wobbleMagnitude;
	float m_wobbleRate;
};

	}
}

