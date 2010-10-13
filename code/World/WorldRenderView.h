#ifndef traktor_world_WorldRenderView_H
#define traktor_world_WorldRenderView_H

#include <string>
#include "Core/Object.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Aabb.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ITexture;
class ProgramParameters;
class Shader;

	}

	namespace world
	{

/*! \brief World render's view.
 * \ingroup World
 *
 * WorldRenderView represent the view of the world from the WorldRenderer's
 * perspective.
 */
class T_DLLCLASS WorldRenderView : public Object
{
	T_RTTI_CLASS;
	
public:
	enum { MaxLightCount = 2 };

	enum LightType
	{
		LtDisabled = 0,
		LtDirectional = 1,
		LtPoint = 2
	};

	struct Light
	{
		LightType type;
		Vector4 position;
		Vector4 direction;
		Vector4 sunColor;
		Vector4 baseColor;
		Vector4 shadowColor;
		Scalar range;
	};

	WorldRenderView();

	void setTechnique(const render::handle_t technique);

	void setViewFrustum(const Frustum& viewFrustum);

	void setCullFrustum(const Frustum& cullFrustum);

	void setProjection(const Matrix44& projection);

	void setView(const Matrix44& view);

	void setViewSize(const Vector2& viewSize);

	void setEyePosition(const Vector4& eyePosition);

	void setShadowBox(const Aabb& shadowBox);

	void setShadowMask(render::ITexture* shadowMask);

	void setDepthMap(render::ITexture* depthMap);

	void setDepthRange(float depthRange);

	void setTimes(float time, float deltaTime, float interval);

	void addLight(const Light& light);

	void resetLights();

	void setShaderTechnique(render::Shader* shader) const;

	void setShaderCombination(render::Shader* shader) const;

	void setShaderCombination(render::Shader* shader, const Matrix44& world, const Aabb& bounds) const;

	/*! \brief Set program parameters defined by world renderer.
	 *
	 * \param programParams Pointer to program parameter container.
	 */
	void setProgramParameters(render::ProgramParameters* programParams) const;

	/*! \brief Set program parameters defined by world renderer.
	 *
	 * \param programParams Pointer to program parameter container.
	 * \param world Entity world transform.
	 * \param bounds Entity bounds in object space.
	 */
	void setProgramParameters(render::ProgramParameters* programParams, const Matrix44& world, const Aabb& bounds) const;

	T_FORCE_INLINE render::handle_t getTechnique() const {
		return m_technique;
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

	T_FORCE_INLINE const Matrix44& getView() const {
		return m_view;
	}

	T_FORCE_INLINE const Vector2& getViewSize() const {
		return m_viewSize;
	}

	T_FORCE_INLINE const Vector4& getEyePosition() const {
		return m_eyePosition;
	}

	T_FORCE_INLINE const Light& getLight(int index) const {
		return m_lights[index];
	}

	T_FORCE_INLINE const Aabb& getShadowBox() const {
		return m_shadowBox;
	}

	T_FORCE_INLINE Ref< render::ITexture > getShadowMask() const {
		return m_shadowMask;
	}

	T_FORCE_INLINE Ref< render::ITexture > getDepthMap() const {
		return m_depthMap;
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
	
private:
	render::handle_t m_technique;
	Frustum m_viewFrustum;
	Frustum m_cullFrustum;
	Matrix44 m_projection;
	Matrix44 m_view;
	Vector2 m_viewSize;
	Vector4 m_eyePosition;
	Light m_lights[MaxLightCount];
	int m_lightCount;
	Aabb m_shadowBox;
	float m_shadowMaskSize;
	Ref< render::ITexture > m_shadowMask;
	float m_depthRange;
	Ref< render::ITexture > m_depthMap;
	float m_time;
	float m_deltaTime;
	float m_interval;

	void setWorldProgramParameters(render::ProgramParameters* programParams, const Matrix44& world) const;

	void setLightProgramParameters(render::ProgramParameters* programParams) const;

	void setLightProgramParameters(render::ProgramParameters* programParams, const Matrix44& world, const Aabb& bounds) const;

	void setShadowMapProgramParameters(render::ProgramParameters* programParams) const;

	void setDepthMapProgramParameters(render::ProgramParameters* programParams) const;
};
	
	}
}

#endif	// traktor_world_WorldRenderView_H
