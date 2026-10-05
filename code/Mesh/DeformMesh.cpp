/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/DeformMesh.h"

#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/IAccelerationStructure.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/Mesh/Mesh.h"
#include "Resource/IResourceManager.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

#include <algorithm>
#include <cstdlib>

namespace traktor::mesh
{
namespace
{

// Compute shader copying undeformed positions of vertices no material deforms.
const resource::Id< render::Shader > c_shaderDeformCopy(L"{2CA77837-CFFF-4259-A98D-8FC2074FF892}");

// Meshes deform within this distance from the eye, the offset fading out over the fade distance
// leading up to it so deforming never starts or stops abruptly; shared by all mesh types.
const float c_deformDistance = 50.0f;
const float c_deformFadeDistance = 4.0f;

// Deformed instances within this distance also refit their ray tracing geometry.
const float c_deformRayTracingDistance = 20.0f;


// Refit the slot acceleration structures this many updates before a full rebuild.
const int32_t c_maxRtUpdatesBeforeBuild = 400;

const render::Handle s_handleDeformVertices(L"Mesh_DeformVertices");
const render::Handle s_handleDeformIndices(L"Mesh_DeformIndices");
const render::Handle s_handleDeformBuffer(L"Mesh_DeformBuffer");
const render::Handle s_handleDeformBufferLast(L"Mesh_DeformBufferLast");
const render::Handle s_handleDeformBufferOutput(L"Mesh_DeformBufferOutput");
const render::Handle s_handleDeformOutputOffset(L"Mesh_DeformOutputOffset");
const render::Handle s_handleDeformIndexOffset(L"Mesh_DeformIndexOffset");
const render::Handle s_handleDeformIndexCount(L"Mesh_DeformIndexCount");
const render::Handle s_handleDeformDistance(L"Mesh_DeformDistance");
const render::Handle s_handleDeformVertexCount(L"Mesh_DeformVertexCount");
const render::Handle s_handleDeformSlot(L"Mesh_DeformSlot");

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.DeformMesh", DeformMesh, IMesh)

const FourCC DeformMesh::c_fccDeformVertices("DFVX");
const FourCC DeformMesh::c_fccDeformIndices("DFIX");

int32_t DeformMesh::allocateDeformSlot(const Object* owner)
{
	T_ASSERT(haveDeform());

	int32_t slot = -1;
	for (int32_t i = 0; i < (int32_t)m_deformSlots.size(); ++i)
	{
		if (m_deformSlots[i].owner == nullptr)
		{
			slot = i;
			break;
		}
	}
	if (slot < 0)
	{
		slot = (int32_t)m_deformSlots.size();
		m_deformSlots.push_back();
		ensureDeformCapacity((uint32_t)m_deformSlots.size());
	}

	DeformSlot& ds = m_deformSlots[slot];
	ds.owner = owner;
	ds.rtUpdates = std::rand() % c_maxRtUpdatesBeforeBuild;
	return slot;
}

void DeformMesh::releaseDeformSlot(int32_t slot)
{
	// Tolerate slots of another (reloaded) mesh; they were never in this pool.
	if (slot < 0 || slot >= (int32_t)m_deformSlots.size())
		return;
	m_deformSlots[slot].owner = nullptr;
	releaseDeformSlotAccelerationStructure(slot);
}

void DeformMesh::beginDeform()
{
	T_ASSERT(m_deformBuffer != nullptr);

	// Pending frames might still read the current deform, thus deform into another allocation;
	// no frame reads a new pool, thus it is deformed in place and is its own history.
	const bool history = (m_lastDeformBufferView != nullptr);
	m_lastDeformBufferView = m_deformBuffer->getBufferView();
	if (history)
		m_deformBuffer->nextFrame();
}

void DeformMesh::buildDeformSlot(
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	int32_t slot,
	const Transform& worldTransform,
	const render::Shader* shader,
	const IMeshParameterCallback* parameterCallback,
	bool writeLast)
{
	T_ASSERT(slot >= 0 && slot < (int32_t)m_deformSlots.size());

	const render::IBufferView* deformBuffer = m_deformBuffer->getBufferView();
	buildDeform(renderContext, worldRenderView, worldTransform, shader, deformBuffer, (uint32_t)slot, parameterCallback);

	// A slot without a valid history gets last frame's positions written as well so
	// velocities start out at zero rather than from whatever the buffer held.
	if (writeLast && m_lastDeformBufferView != deformBuffer)
		buildDeform(renderContext, worldRenderView, worldTransform, shader, m_lastDeformBufferView, (uint32_t)slot, parameterCallback);
}

const render::IAccelerationStructure* DeformMesh::buildDeformSlotAccelerationStructure(
	render::RenderContext* renderContext,
	int32_t slot)
{
	T_ASSERT(slot >= 0 && slot < (int32_t)m_deformSlots.size());

	// Created from the undeformed source positions; a full build first so refits start from the deformed geometry.
	DeformSlot& ds = m_deformSlots[slot];
	bool rebuild = false;
	if (ds.blas == nullptr)
	{
		ds.blas = createDeformAccelerationStructure();
		if (ds.blas == nullptr)
			return nullptr;
		rebuild = true;
	}
	if (++ds.rtUpdates > c_maxRtUpdatesBeforeBuild)
	{
		rebuild = true;
		ds.rtUpdates = 0;
	}

	const bool asynchronous = renderContext->isAsyncCompute();

	if (!asynchronous)
		renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::AccelerationStructureUpdate, nullptr, 0);

	// The slot's positions sit at an offset into the buffer; address them through the
	// primitives' first vertex rather than a buffer view.
	AlignedVector< render::RaytracingPrimitives > primitives = m_renderMesh->getRaytracingPrimitives();
	for (auto& rtp : primitives)
		rtp.firstVertex = (uint32_t)slot * m_deformVertexCount;

	// Refit from this frame's deform into a structure which no pending frame reads.
	const render::IBufferView* deformBuffer = m_deformBuffer->getBufferView();
	render::IAccelerationStructure* accelerationStructure = ds.blas;
	accelerationStructure->nextFrame();

	auto rb = renderContext->allocNamed< render::LambdaRenderBlock >(L"Mesh deform AS");
	rb->lambda = [=, this, primitives = std::move(primitives)](render::IRenderView* renderView) {
		renderView->writeAccelerationStructure(
			accelerationStructure,
			deformBuffer,
			m_deformVertexLayout,
			m_renderMesh->getIndexBuffer()->getBufferView(),
			m_renderMesh->getIndexType(),
			primitives,
			rebuild,
			asynchronous);
	};
	renderContext->compute(rb);

	return ds.blas;
}

void DeformMesh::releaseDeformSlotAccelerationStructure(int32_t slot)
{
	if (slot < 0 || slot >= (int32_t)m_deformSlots.size())
		return;

	// Destroyed rather than kept for reuse; the structures are what the pool's memory is made of.
	DeformSlot& ds = m_deformSlots[slot];
	if (ds.blas)
	{
		ds.blas->destroy();
		ds.blas = nullptr;
	}
}

void DeformMesh::setDeformParameters(render::ProgramParameters* programParams, int32_t slot)
{
	T_ASSERT(haveDeform());

	// Deforming vertex fragments declare the buffers, thus a placeholder is bound while no instance
	// holds a slot; undeformed draws never read from it.
	const render::IBufferView* deformBuffer = nullptr;
	const render::IBufferView* lastDeformBuffer = nullptr;
	if (m_deformBuffer != nullptr)
	{
		deformBuffer = m_deformBuffer->getBufferView();
		lastDeformBuffer = (m_lastDeformBufferView != nullptr) ? m_lastDeformBufferView : deformBuffer;
	}
	else
	{
		if (m_deformPlaceholder == nullptr)
			m_deformPlaceholder = m_renderSystem->createBuffer(render::BuStructured, sizeof(DeformPosition), false, T_FILE_LINE_W);
		deformBuffer = lastDeformBuffer = m_deformPlaceholder->getBufferView();
	}

	programParams->setBufferViewParameter(s_handleDeformBuffer, deformBuffer);
	programParams->setBufferViewParameter(s_handleDeformBufferLast, lastDeformBuffer);
	programParams->setFloatParameter(s_handleDeformVertexCount, (float)m_deformVertexCount + 0.5f);
	programParams->setFloatParameter(s_handleDeformSlot, (float)slot);
}

float DeformMesh::getDeformDistance()
{
	return c_deformDistance;
}

float DeformMesh::getDeformFadeDistance()
{
	return c_deformFadeDistance;
}

float DeformMesh::getDeformRayTracingDistance()
{
	return c_deformRayTracingDistance;
}

namespace
{

Scalar distanceToBox(const Transform& worldTransform, const Aabb3& boundingBox, const Vector4& eyePosition)
{
	const Aabb3 worldBoundingBox = boundingBox.transform(worldTransform);
	const Vector4 closest = max(worldBoundingBox.mn, min(worldBoundingBox.mx, eyePosition));
	return (closest - eyePosition).xyz0().length();
}

}

bool DeformMesh::isWithinDeformDistance(const Transform& worldTransform, const Aabb3& boundingBox, const Vector4& eyePosition)
{
	return distanceToBox(worldTransform, boundingBox, eyePosition) < Scalar(c_deformDistance);
}

bool DeformMesh::isWithinDeformRayTracingDistance(const Transform& worldTransform, const Aabb3& boundingBox, const Vector4& eyePosition)
{
	return distanceToBox(worldTransform, boundingBox, eyePosition) < Scalar(c_deformRayTracingDistance);
}

bool DeformMesh::createDeform(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const AlignedVector< MeshResource::DeformPart >& deformParts,
	render::Mesh* renderMesh)
{
	m_deformParts.resize(0);
	if (deformParts.empty())
		return true;

	if (!resourceManager->bind(c_shaderDeformCopy, m_shaderDeformCopy))
		return false;

	const render::Buffer* deformVertices = renderMesh->getAuxBuffer(c_fccDeformVertices);
	if (deformVertices == nullptr || renderMesh->getAuxBuffer(c_fccDeformIndices) == nullptr)
	{
		log::error << L"Mesh deform create failed; missing deform buffers." << Endl;
		return false;
	}

	for (const MeshResource::DeformPart& resourcePart : deformParts)
	{
		DeformPart& part = m_deformParts.push_back();
		part.shaderTechnique = !resourcePart.shaderTechnique.empty() ? render::getParameterHandle(resourcePart.shaderTechnique) : 0;
		part.indexOffset = resourcePart.indexOffset;
		part.indexCount = resourcePart.indexCount;
	}

	m_deformVertexLayout = renderSystem->createVertexLayout({
		render::VertexElement(render::DataUsage::Position, render::DtFloat4, offsetof(DeformPosition, Position)) });
	if (!m_deformVertexLayout)
	{
		log::error << L"Mesh deform create failed; unable to create deform vertex layout." << Endl;
		return false;
	}

	m_renderMesh = renderMesh;
	m_renderSystem = renderSystem;
	m_deformVertexCount = deformVertices->getBufferSize() / sizeof(DeformVertex);

	log::info << L"Mesh deform; " << m_deformVertexCount << L" vertices, " << (m_deformVertexCount * sizeof(DeformPosition)) / 1024 << L" KiB per deform slot and pooled allocation." << Endl;
	return true;
}

void DeformMesh::ensureDeformCapacity(uint32_t slotCount)
{
	if (slotCount <= m_deformCapacity)
		return;

	// Recreate the pool, losing every slot's history; grown geometrically but a slot
	// is a lot of memory for a detailed mesh, thus not far beyond what is in use.
	const uint32_t capacity = std::max< uint32_t >(slotCount, std::min< uint32_t >(m_deformCapacity * 2, slotCount + 8));
	m_deformBuffer = m_renderSystem->createBuffer(render::BuStructured | render::BuPooled, capacity * m_deformVertexCount * sizeof(DeformPosition), false, T_FILE_LINE_W);
	m_lastDeformBufferView = nullptr;
	m_deformCapacity = capacity;
}

void DeformMesh::buildDeform(
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	const Transform& worldTransform,
	const render::Shader* shader,
	const render::IBufferView* deformBuffer,
	uint32_t slot,
	const IMeshParameterCallback* parameterCallback) const
{
	const bool asynchronous = renderContext->isAsyncCompute();
	const render::Buffer* deformVertices = m_renderMesh->getAuxBuffer(c_fccDeformVertices);
	const render::Buffer* deformIndices = m_renderMesh->getAuxBuffer(c_fccDeformIndices);

	// Parameters shared by all parts; the surface graph evaluating the offsets may read anything
	// a vertex shader can, thus the frame and object parameters of a render pass are set here.
	const Matrix44 world = worldTransform.toMatrix44();
	const Vector4 deformDistance(c_deformDistance, 1.0f / std::max(c_deformFadeDistance, 0.001f), 0.0f, 0.0f);

	auto sharedParams = renderContext->alloc< render::ProgramParameters >();
	sharedParams->beginParameters(renderContext);
	sharedParams->setFloatParameter(world::ShaderParameter::Time, (float)worldRenderView.getTime());
	sharedParams->setMatrixParameter(world::ShaderParameter::Projection, worldRenderView.getProjection());
	sharedParams->setMatrixParameter(world::ShaderParameter::View, worldRenderView.getView());
	sharedParams->setMatrixParameter(world::ShaderParameter::ViewInverse, worldRenderView.getView().inverse());
	sharedParams->setMatrixParameter(world::ShaderParameter::World, world);
	sharedParams->setMatrixParameter(world::ShaderParameter::WorldView, worldRenderView.getView() * world);
	if (parameterCallback)
		parameterCallback->setParameters(sharedParams);
	sharedParams->setBufferViewParameter(s_handleDeformVertices, deformVertices->getBufferView());
	sharedParams->setBufferViewParameter(s_handleDeformIndices, deformIndices->getBufferView());
	sharedParams->setBufferViewParameter(s_handleDeformBufferOutput, deformBuffer);
	sharedParams->setFloatParameter(s_handleDeformOutputOffset, (float)(slot * m_deformVertexCount));
	sharedParams->setVectorParameter(s_handleDeformDistance, deformDistance);
	sharedParams->endParameters(renderContext);

	// Dispatch each part over its range of the vertex index list; together the parts
	// cover every vertex so the slot is completely written.
	for (const auto& part : m_deformParts)
	{
		render::IProgram* program = nullptr;
		if (part.shaderTechnique != 0)
		{
			const auto sp = shader->getProgram(render::Shader::Permutation(part.shaderTechnique));
			if (!sp)
				continue;
			program = sp.program;
		}
		else
			program = m_shaderDeformCopy->getProgram().program;

		auto programParams = renderContext->alloc< render::ProgramParameters >();
		programParams->beginParameters(renderContext);
		programParams->attachParameters(sharedParams);
		programParams->setFloatParameter(s_handleDeformIndexOffset, (float)part.indexOffset);
		programParams->setFloatParameter(s_handleDeformIndexCount, (float)part.indexCount + 0.5f);
		programParams->endParameters(renderContext);

		auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Mesh deform");
		renderBlock->program = program;
		renderBlock->programParams = programParams;
		renderBlock->workSize[0] = (int32_t)part.indexCount;

		renderContext->compute(renderBlock);
	}

	if (!asynchronous)
		renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Vertex, nullptr, 0);
}

Ref< render::IAccelerationStructure > DeformMesh::createDeformAccelerationStructure() const
{
	// No acceleration structure when ray tracing is unsupported, or when the mesh was built
	// with ray tracing disabled (in which case it carries no ray tracing primitives).
	if (!m_renderSystem->supportRayTracing() || m_renderMesh->getRaytracingPrimitives().empty())
		return nullptr;

	// Initially built from the undeformed source positions; refit from the deform buffers
	// afterwards. The layout carry every element of the source vertex so the pitch is correct.
	Ref< const render::IVertexLayout > sourceLayout = m_renderSystem->createVertexLayout({
		render::VertexElement(render::DataUsage::Position, render::DtFloat4, offsetof(DeformVertex, Position)),
		render::VertexElement(render::DataUsage::Normal, render::DtHalf4, offsetof(DeformVertex, Normal)),
		render::VertexElement(render::DataUsage::Tangent, render::DtHalf4, offsetof(DeformVertex, Tangent)),
		render::VertexElement(render::DataUsage::Binormal, render::DtHalf4, offsetof(DeformVertex, Binormal)),
		render::VertexElement(render::DataUsage::Custom, render::DtFloat2, offsetof(DeformVertex, TexCoord0), 0),
		render::VertexElement(render::DataUsage::Custom, render::DtFloat2, offsetof(DeformVertex, TexCoord1), 1),
		render::VertexElement(render::DataUsage::Color, render::DtHalf4, offsetof(DeformVertex, Color)) });
	if (!sourceLayout)
		return nullptr;

	return m_renderSystem->createAccelerationStructure(
		m_renderMesh->getAuxBuffer(c_fccDeformVertices),
		sourceLayout,
		m_renderMesh->getIndexBuffer(),
		m_renderMesh->getIndexType(),
		m_renderMesh->getRaytracingPrimitives(),
		true,
		true);
}

}
