#ifndef traktor_world_WorldRenderView_H
#define traktor_world_WorldRenderView_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Vector2.h"
#include "World/WorldTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief World render view.
 * \ingroup World
 *
 * WorldRenderView represent the view of the world from the WorldRenderer's
 * perspective.
 */
class T_DLLCLASS WorldRenderView : public Object
{
	T_RTTI_CLASS;
	
public:
	WorldRenderView();

	void setIndex(int32_t index);

	void setOrthogonal(float width, float height, float nearZ, float farZ);

	void setPerspective(float width, float height, float aspect, float fov, float nearZ, float farZ);

	void setViewFrustum(const Frustum& viewFrustum);

	void setCullFrustum(const Frustum& cullFrustum);

	void setProjection(const Matrix44& projection);

	void setView(const Matrix44& lastView, const Matrix44& view);

	void setViewSize(const Vector2& viewSize);

	void setShadowBox(const Aabb3& shadowBox);

	void setTimes(float time, float deltaTime, float interval);

	void setEyePosition(const Vector4& eyePosition);

	void setEyeDirection(const Vector4& eyeDirection);

	void setInterocularDistance(float interocularDistance);

	void setDistortionValue(float distortionValue);

	void setScreenPlaneDistance(float screenPlaneDistance);

	void setGodRayDirection(const Vector4& direction);

	void addLight(const Light& light);

	void resetLights();

	T_FORCE_INLINE int getIndex() const {
		return m_index;
	}

	T_FORCE_INLINE const Frustum& getViewFrustum() const {
		return m_viewFrustum;
	}

	T_FORCE_INLINE const Frustum& getCullFrustum() const {
		return m_cullFrustum;
	}

	T_FORCE_INLINE const Matrix44& getProjection() const {
		return m_projection;
	}

	T_FORCE_INLINE const Matrix44& getLastView() const {
		return m_lastView;
	}

	T_FORCE_INLINE const Matrix44& getView() const {
		return m_view;
	}

	T_FORCE_INLINE const Vector2& getViewSize() const {
		return m_viewSize;
	}

	T_FORCE_INLINE const Light& getLight(int index) const {
		return m_lights[index];
	}

	T_FORCE_INLINE int32_t getLightCount() const {
		return int32_t(m_lights.size());
	}

	T_FORCE_INLINE const Aabb3& getShadowBox() const {
		return m_shadowBox;
	}

	T_FORCE_INLINE float getTime() const {
		return m_time;
	}

	T_FORCE_INLINE float getDeltaTime() const {
		return m_deltaTime;
	}

	T_FORCE_INLINE float getInterval() const {
		return m_interval;
	}

	T_FORCE_INLINE const Vector4& getEyePosition() const {
		return m_eyePosition;
	}

	T_FORCE_INLINE const Vector4& getEyeDirection() const {
		return m_eyeDirection;
	}

	T_FORCE_INLINE float getInterocularDistance() const {
		return m_interocularDistance;
	}

	T_FORCE_INLINE float getDistortionValue() const {
		return m_distortionValue;
	}

	T_FORCE_INLINE float getScreenPlaneDistance() const {
		return m_screenPlaneDistance;
	}

	T_FORCE_INLINE const Vector4& getGodRayDirection() const {
		return m_godRayDirection;
	}

private:
	int32_t m_index;
	Frustum m_viewFrustum;
	Frustum m_cullFrustum;
	Matrix44 m_projection;
	Matrix44 m_lastView;
	Matrix44 m_view;
	Vector2 m_viewSize;
	AlignedVector< Light > m_lights;
	Aabb3 m_shadowBox;
	float m_time;
	float m_deltaTime;
	float m_interval;
	Vector4 m_eyePosition;
	Vector4 m_eyeDirection;
	float m_interocularDistance;
	float m_distortionValue;
	float m_screenPlaneDistance;
	Vector4 m_godRayDirection;
};
	
	}
}

#endif	// traktor_world_WorldRenderView_H
