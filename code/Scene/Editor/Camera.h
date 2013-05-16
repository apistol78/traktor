#ifndef traktor_scene_Camera_H
#define traktor_scene_Camera_H

#include "Core/Object.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class EntityAdapter;

/*! \brief Preview render camera. */
class T_DLLCLASS Camera : public Object
{
public:
	Camera();

	void setEnable(bool enable);

	bool isEnable() const;

	void place(const Vector4& position);

	void move(const Vector4& direction);

	void rotate(float dy, float dx);

	void setFollowEntity(EntityAdapter* followEntity);

	void setLookAtEntity(EntityAdapter* lookAtEntity);

	Matrix44 getWorld() const;

	Matrix44 getView() const;

	void setPosition(const Vector4& position) { m_position = position; }

	const Vector4& getPosition() const { return m_position; }

	void setOrientation(const Quaternion& orientation) { m_orientation = orientation; }

	const Quaternion& getOrientation() const { return m_orientation; }

private:
	bool m_enable;
	Vector4 m_position;
	Quaternion m_orientation;
	Ref< EntityAdapter > m_followEntity;
	Ref< EntityAdapter > m_lookAtEntity;
};

	}
}

#endif	// traktor_scene_Camera_H
