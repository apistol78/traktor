/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Half.h"
#include "Core/Math/Transform.h"
#include "Core/Ref.h"
#include "Mesh/IMesh.h"
#include "Mesh/MeshResource.h"
#include "Render/Shader.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class Buffer;
class IAccelerationStructure;
class IBufferView;
class IRenderSystem;
class IVertexLayout;
class Mesh;
class ProgramParameters;
class RenderContext;

}

namespace traktor::resource
{

class IResourceManager;

}

namespace traktor::world
{

class WorldRenderView;

}

namespace traktor::mesh
{

class IMeshParameterCallback;

/*! Deformable mesh.
 * \ingroup Mesh
 *
 * World position offsets are evaluated per vertex on the compute queue into a deform buffer, a slot per instance near the eye.
 */
class T_DLLCLASS DeformMesh : public IMesh
{
	T_RTTI_CLASS;

public:
	/*! Deform source vertices; attributes of every vertex in the DeformVertex layout, read by the Deform compute technique. */
	static const FourCC c_fccDeformVertices;

	/*! Deform vertex indices; the concatenated vertex index lists of the deform parts. */
	static const FourCC c_fccDeformIndices;

#pragma pack(1)
	/*! Deform source vertex; layout of the c_fccDeformVertices aux buffer, must match the Mesh_DeformVertex_Type struct declaration. */
	struct DeformVertex
	{
		float Position[4];
		half_t Normal[4];
		half_t Tangent[4];
		half_t Binormal[4];
		float TexCoord0[2];
		float TexCoord1[2];
		half_t Color[4];
	};

	/*! Deformed vertex; layout of the deform buffers, must match the Mesh_DeformPosition_Type struct declaration. */
	struct DeformPosition
	{
		float Position[4];
	};
#pragma pack()

	/*! Deform part; a range of the deform vertex index list dispatched with a compute technique. */
	struct DeformPart
	{
		render::handle_t shaderTechnique; //!< Deform technique of the material, or 0 to copy undeformed positions.
		uint32_t indexOffset;
		uint32_t indexCount;
	};

	/*! Check if any material of this mesh deforms. */
	bool haveDeform() const { return !m_deformParts.empty(); }

	/*! Number of vertices in the deform buffers, i.e. the size of one deform slot. */
	uint32_t getDeformVertexCount() const { return m_deformVertexCount; }

	//! \name Deform slots
	//! \{

	/*! Allocate a deform slot for an instance; the pool grows as required. */
	int32_t allocateDeformSlot(const Object* owner);

	/*! Release a deform slot. */
	void releaseDeformSlot(int32_t slot);

	/*! Begin deforming this frame; moves the deform buffer to an allocation no pending frame reads. Call before building slots. */
	void beginDeform();

	/*! Build compute work deforming an instance into its slot, using the deform techniques of the material shader.
	 * writeLast also writes the previous frame's buffer, for slots without a valid history.
	 */
	void buildDeformSlot(
		render::RenderContext* renderContext,
		const world::WorldRenderView& worldRenderView,
		int32_t slot,
		const Transform& worldTransform,
		const render::Shader* shader,
		const IMeshParameterCallback* parameterCallback,
		bool writeLast);

	/*! Build update of the slot's acceleration structure from its deformed positions; null when the mesh isn't ray traced. */
	const render::IAccelerationStructure* buildDeformSlotAccelerationStructure(
		render::RenderContext* renderContext,
		int32_t slot);

	/*! Release the slot's acceleration structure; the instance is traced through the shared, undeformed, one. */
	void releaseDeformSlotAccelerationStructure(int32_t slot);

	/*! Set the deform parameters of a draw; slot is the one the drawn instance holds, or -1 when drawn undeformed. */
	void setDeformParameters(render::ProgramParameters* programParams, int32_t slot);

	//! \}

	/*! Distance from the eye within which meshes deform; shared by all mesh types. */
	static float getDeformDistance();

	/*! Distance over which the offset fades out leading up to the deform distance. */
	static float getDeformFadeDistance();

	/*! Distance from the eye within which deformed instances also refit their ray tracing geometry.
	 * A structure per instance is far more memory than the deform, thus farther ones trace their undeformed geometry.
	 */
	static float getDeformRayTracingDistance();

	/*! Check if any part of the mesh instance's bounding box is within deform distance of the eye.
	 * Vertices beyond the distance are faded out by the deform, thus deforming stops once the whole box is beyond.
	 */
	static bool isWithinDeformDistance(const Transform& worldTransform, const Aabb3& boundingBox, const Vector4& eyePosition);

	/*! Check if a mesh instance is within ray tracing deform distance of the eye. */
	static bool isWithinDeformRayTracingDistance(const Transform& worldTransform, const Aabb3& boundingBox, const Vector4& eyePosition);

protected:
	/*! Create deform from resource parts; binds the copy shader and derives the vertex count and layout. */
	bool createDeform(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		const AlignedVector< MeshResource::DeformPart >& deformParts,
		render::Mesh* renderMesh);

private:
	struct DeformSlot
	{
		const Object* owner = nullptr;
		Ref< render::IAccelerationStructure > blas;
		int32_t rtUpdates = 0;
		bool rtRebuild = false; //!< Geometry has changed, thus next update is a full build.
	};

	AlignedVector< DeformPart > m_deformParts;
	resource::Proxy< render::Shader > m_shaderDeformCopy;
	Ref< const render::IVertexLayout > m_deformVertexLayout;
	Ref< render::Mesh > m_renderMesh;
	Ref< render::IRenderSystem > m_renderSystem;
	uint32_t m_deformVertexCount = 0;

	Ref< render::Buffer > m_deformBuffer;						//!< Pooled; holding every slot.
	const render::IBufferView* m_lastDeformBufferView = nullptr; //!< Deform of the previous frame, read for velocities.
	Ref< render::Buffer > m_deformPlaceholder;					//!< Bound instead of the pool while none exists.
	AlignedVector< DeformSlot > m_deformSlots;
	uint32_t m_deformCapacity = 0;
	uint32_t m_deformWrites = 0;
	bool m_deformWriteLastAll = false;		//!< The pool was recreated; every slot must rewrite its history.
	bool m_deformWriteLastAllFrame = false; //!< Rewrite histories while building this frame's slots.

	/*! Ensure the deform pool holds at least the given number of slots. */
	void ensureDeformCapacity(uint32_t slotCount);

	/*! Build compute work deforming every vertex into a slot of a buffer. */
	void buildDeform(
		render::RenderContext* renderContext,
		const world::WorldRenderView& worldRenderView,
		const Transform& worldTransform,
		const render::Shader* shader,
		const render::IBufferView* deformBuffer,
		uint32_t slot,
		const IMeshParameterCallback* parameterCallback) const;

	/*! Create an acceleration structure which can be refit from a deform buffer. */
	Ref< render::IAccelerationStructure > createDeformAccelerationStructure() const;
};

}
