#ifndef traktor_scene_Camera_H
#define traktor_scene_Camera_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

/*! \brief Preview render camera. */
class T_DLLCLASS Camera : public Object
{
public:
	Camera(const Matrix44& transform);

	void setEnable(bool enable);

	bool isEnable() const;

	void enterFreeLook();

	void enterLookAt(const Vector4& lookAtPosition);

	void place(const Matrix44& transform);

	void move(const Vector4& direction);

	void rotate(float dy, float dx);

	void setTargetView(const Matrix44& transform);

	void update(float deltaTime);

	Matrix44 getCurrentWorld() const;

	Matrix44 getTargetWorld() const;

	Matrix44 getCurrentView() const;

	Matrix44 getTargetView() const;

	inline void setCurrentPosition(const Vector4& position) { m_current.position = position; }

	inline const Vector4& getCurrentPosition() const { return m_current.position; }

	inline void setCurrentOrientation(const Quaternion& orientation) { m_current.orientation = orientation; }

	inline const Quaternion& getCurrentOrientation() const { return m_current.orientation; }

	inline void setTargetPosition(const Vector4& position) { m_target.position = position; }

	inline const Vector4& getTargetPosition() const { return m_target.position; }

	inline void setTargetOrientation(const Quaternion& orientation) { m_target.orientation = orientation; }

	inline const Quaternion& getTargetOrientation() const { return m_target.orientation; }

private:
	enum LookMode
	{
		LmFree,
		LmLookAt
	};

	struct Frame
	{
		Vector4 position;
		Quaternion orientation;
	};

	bool m_enable;
	Frame m_current;
	Frame m_target;
	LookMode m_lookMode;
	Vector4 m_lookAtPosition;
	Scalar m_lookAtDistance;
};

	}
}

#endif	// traktor_scene_Camera_H
