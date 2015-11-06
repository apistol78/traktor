#ifndef traktor_animation_AnimatedMeshEntity_H
#define traktor_animation_AnimatedMeshEntity_H

#include "Animation/Pose.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Thread/Job.h"
#include "Resource/Proxy.h"
#include "Mesh/MeshEntity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class SkinnedMesh;

	}

	namespace animation
	{

class Skeleton;
class IPoseController;

/*! \brief Animated mesh entity.
 * \ingroup Animation
 */
class T_DLLCLASS AnimatedMeshEntity : public mesh::MeshEntity
{
	T_RTTI_CLASS;

public:
	struct Binding
	{
		render::handle_t jointHandle;
		Ref< world::Entity > entity;
	};

	AnimatedMeshEntity(
		const Transform& transform,
		const resource::Proxy< mesh::SkinnedMesh >& mesh,
		const resource::Proxy< Skeleton >& skeleton,
		IPoseController* poseController,
		const std::vector< int32_t >& jointRemap,
		const std::vector< Binding >& bindings,
		bool normalizePose,
		bool normalizeTransform,
		bool screenSpaceCulling
	);

	virtual ~AnimatedMeshEntity();

	virtual void destroy() T_OVERRIDE T_FINAL;
	
	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual bool supportTechnique(render::handle_t technique) const T_OVERRIDE T_FINAL;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	) T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	bool getJointTransform(render::handle_t jointName, Transform& outTransform) const;

	bool getPoseTransform(render::handle_t jointName, Transform& outTransform) const;

	bool getSkinTransform(render::handle_t jointName, Transform& outTransform) const;

	bool setPoseTransform(render::handle_t jointName, const Transform& transform, bool inclusive);

	const resource::Proxy< Skeleton >& getSkeleton() const { return m_skeleton; }

	void setPoseController(IPoseController* poseController) { m_poseController = poseController; }

	const Ref< IPoseController >& getPoseController() const { return m_poseController; }

	const AlignedVector< Transform >& getJointTransforms() const { return m_jointTransforms; }

	const AlignedVector< Transform >& getPoseTransforms() const { return m_poseTransforms; }

private:
	resource::Proxy< mesh::SkinnedMesh > m_mesh;
	resource::Proxy< Skeleton > m_skeleton;
	Ref< IPoseController > m_poseController;
	std::vector< int32_t > m_jointRemap;
	std::vector< Binding > m_bindings;
	bool m_normalizePose;
	bool m_normalizeTransform;
	AlignedVector< Transform > m_jointTransforms;
	AlignedVector< Transform > m_poseTransforms;
	AlignedVector< Vector4 > m_skinTransforms[3];
	int32_t m_index;
	int32_t m_updateController;
	float m_updateTimeScale;
	mutable Ref< Job > m_updatePoseControllerJob;

	void synchronize() const;

	void updatePoseController(int32_t index, float deltaTime);
};

	}
}

#endif	// traktor_animation_AnimatedMeshEntity_H
