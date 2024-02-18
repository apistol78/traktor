/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
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

namespace traktor::world
{

/*! World render view.
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

	/*! Set view index.
	 *
	 * Index is used to differentiate, for example,
	 * if split screen is used etc.
	 *
	 * It's a way for renderers to keep frame to frame
	 * caching schemes separate for each view.
	 */
	void setIndex(int32_t index);

	/*! */
	void setCascade(int32_t cascade);

	/*! Set snapshot requirement.
	 *
	 * Snapshot is used, for example, when rendering
	 * reflection probes etc. No frame caching can 
	 * be used, especially those who require multiple
	 * frames to "catch up".
	 */
	void setSnapshot(bool snapshot);

	void setOrthogonal(float width, float height, float nearZ, float farZ);

	void setPerspective(float width, float height, float aspect, float fov, float nearZ, float farZ);

	void setViewFrustum(const Frustum& viewFrustum);

	void setCullFrustum(const Frustum& cullFrustum);

	void setProjection(const Matrix44& projection);

	void setView(const Matrix44& lastView, const Matrix44& view);

	void setViewSize(const Vector2& viewSize);

	void setTimes(double time, double deltaTime, float interval);

	/*!
	 */
	bool isBoxVisible(const Aabb3& box, const Transform& worldTransform, float& outDistance) const;

	T_FORCE_INLINE int32_t getIndex() const {
		return m_index;
	}

	T_FORCE_INLINE int32_t getCascade() const {
		return m_cascade;
	}

	T_FORCE_INLINE bool getSnapshot() const {
		return m_snapshot;
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

	T_FORCE_INLINE double getTime() const {
		return m_time;
	}

	T_FORCE_INLINE double getDeltaTime() const {
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

private:
	int32_t m_index;
	int32_t m_cascade;
	bool m_snapshot;
	Frustum m_viewFrustum;
	Frustum m_cullFrustum;
	Matrix44 m_projection;
	Matrix44 m_lastView;
	Matrix44 m_view;
	Vector2 m_viewSize;
	double m_time;
	double m_deltaTime;
	float m_interval;
	Vector4 m_eyePosition;
	Vector4 m_eyeDirection;
};

}
