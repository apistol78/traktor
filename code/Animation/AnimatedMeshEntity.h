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
	AnimatedMeshEntity(
		const Transform& transform,
		const resource::Proxy< mesh::SkinnedMesh >& mesh,
		const resource::Proxy< Skeleton >& skeleton,
		IPoseController* poseController,
		const std::vector< int >& boneRemap,
		bool normalizePose,
		bool normalizeTransform
	);

	virtual ~AnimatedMeshEntity();

	virtual void destroy();
	
	virtual Aabb3 getBoundingBox() const;

	virtual bool supportTechnique(render::handle_t technique) const;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	);

	virtual void update(const world::EntityUpdate* update);

	virtual void setTransform(const Transform& transform);

	bool getBoneTransform(const std::wstring& boneName, Transform& outTransform) const;

	bool getPoseTransform(const std::wstring& boneName, Transform& outTransform) const;

	bool getSkinTransform(const std::wstring& boneName, Transform& outTransform) const;

	bool setNeutralPoseTransform(const std::wstring& boneName, const Transform& transform);

	const resource::Proxy< Skeleton >& getSkeleton() const { return m_skeleton; }

	void setPoseController(IPoseController* poseController) { m_poseController = poseController; }

	const Ref< IPoseController >& getPoseController() const { return m_poseController; }

	const AlignedVector< Transform >& getBoneTransforms() const { return m_boneTransforms; }

	const AlignedVector< Transform >& getPoseTransforms() const { return m_poseTransforms; }

private:
	mutable resource::Proxy< mesh::SkinnedMesh > m_mesh;
	mutable resource::Proxy< Skeleton > m_skeleton;
	Ref< Pose > m_neutralPose;
	Ref< IPoseController > m_poseController;
	std::vector< int32_t > m_boneRemap;
	bool m_normalizePose;
	bool m_normalizeTransform;
	AlignedVector< Transform > m_boneTransforms;
	AlignedVector< Transform > m_poseTransforms;
	AlignedVector< Vector4 > m_skinTransforms;
	float m_totalTime;
	bool m_updateController;
	mutable Ref< Job > m_updatePoseControllerJob;

	void synchronize() const;

	void updatePoseController(float deltaTime);
};

	}
}

#endif	// traktor_animation_AnimatedMeshEntity_H
