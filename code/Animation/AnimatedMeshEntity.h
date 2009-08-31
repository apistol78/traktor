#ifndef traktor_animation_AnimatedMeshEntity_H
#define traktor_animation_AnimatedMeshEntity_H

#include "Core/Containers/AlignedVector.h"
#include "Mesh/MeshEntity.h"
#include "Animation/Pose.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(AnimatedMeshEntity)

public:
	AnimatedMeshEntity(
		const Matrix44& transform,
		const resource::Proxy< mesh::SkinnedMesh >& mesh,
		const resource::Proxy< Skeleton >& skeleton,
		IPoseController* poseController,
		const std::vector< int >& boneRemap
	);
	
	virtual Aabb getBoundingBox() const;

	virtual void render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance);

	virtual void update(const world::EntityUpdate* update);

	bool getBoneTransform(const std::wstring& boneName, Matrix44& outTransform) const;

	bool getPoseTransform(const std::wstring& boneName, Matrix44& outTransform) const;

	bool getSkinTransform(const std::wstring& boneName, Matrix44& outTransform) const;

	inline const resource::Proxy< Skeleton >& getSkeleton() const { return m_skeleton; }

	inline void setPoseController(IPoseController* poseController) { m_poseController = poseController; }

	inline const Ref< IPoseController >& getPoseController() const { return m_poseController; }

	inline const AlignedVector< Matrix44 >& getBoneTransforms() const { return m_boneTransforms; }

	inline const AlignedVector< Matrix44 >& getPoseTransforms() const { return m_poseTransforms; }

private:
	mutable resource::Proxy< mesh::SkinnedMesh > m_mesh;
	mutable resource::Proxy< Skeleton > m_skeleton;
	Ref< IPoseController > m_poseController;
	std::vector< int > m_boneRemap;
	AlignedVector< Matrix44 > m_boneTransforms;
	AlignedVector< Matrix44 > m_poseTransforms;
	AlignedVector< Matrix44 > m_skinTransforms;
	float m_totalTime;
	bool m_updateController;
};

	}
}

#endif	// traktor_animation_AnimatedMeshEntity_H
