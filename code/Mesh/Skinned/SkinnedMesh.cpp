/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"

namespace traktor::mesh
{
	namespace
	{

const render::Handle s_handleSkinBuffer(L"Mesh_SkinBuffer");
const render::Handle s_handleSkinBufferLast(L"Mesh_SkinBufferLast");
const render::Handle s_handleSkinBufferOutput(L"Mesh_SkinBufferOutput");
const render::Handle s_handleJoints(L"Mesh_Joints");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.SkinnedMesh", SkinnedMesh, IMesh)

const FourCC SkinnedMesh::c_fccSkinPosition("SPOS");

const Aabb3& SkinnedMesh::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool SkinnedMesh::supportTechnique(render::handle_t technique) const
{
	return m_parts.find(technique) != m_parts.end();
}

void SkinnedMesh::buildSkin(
	render::RenderContext* renderContext,
	render::Buffer* jointTransforms,
	render::Buffer* skinBuffer
) const
{
	const uint32_t vertexCount = m_mesh->getAuxBuffer(c_fccSkinPosition)->getBufferSize() / (6 * 4 * sizeof(float));

	auto programParams = renderContext->alloc< render::ProgramParameters >();
	programParams->beginParameters(renderContext);
	programParams->setBufferViewParameter(s_handleSkinBuffer, m_mesh->getAuxBuffer(c_fccSkinPosition)->getBufferView());
	programParams->setBufferViewParameter(s_handleSkinBufferOutput, skinBuffer->getBufferView());
	programParams->setBufferViewParameter(s_handleJoints, jointTransforms->getBufferView());
	programParams->endParameters(renderContext);

	auto renderBlock = renderContext->alloc< render::ComputeRenderBlock >();
	renderBlock->program = m_shaderUpdateSkin->getProgram().program;
	renderBlock->programParams = programParams;
	renderBlock->workSize[0] = vertexCount;
	renderContext->compute(renderBlock);

	renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Vertex, nullptr, 0);
}

void SkinnedMesh::buildAccelerationStructure(
	render::RenderContext* renderContext,
	render::Buffer* skinBuffer,
	render::IAccelerationStructure* accelerationStructure
) const
{
	// Wait for data to be ready for building AS.
	renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::AccelerationStructureUpdate, nullptr, 0);

	// Rebuild acceleration structure.
	auto rb = renderContext->allocNamed< render::LambdaRenderBlock >(L"AnimatedMeshComponent update AS");
	rb->lambda = [=, this](render::IRenderView* renderView)
	{
		const auto& part = m_mesh->getParts().back();
		T_FATAL_ASSERT(part.name == L"__RT__");

		AlignedVector< render::Primitives > primitives;
		primitives.push_back(part.primitives);

		renderView->writeAccelerationStructure(
			accelerationStructure,
			skinBuffer->getBufferView(),
			m_rtVertexLayout,
			m_mesh->getIndexBuffer()->getBufferView(),
			m_mesh->getIndexType(),
			primitives
		);
	};
	renderContext->compute(rb);
}

void SkinnedMesh::build(
	render::RenderContext* renderContext,
	const world::IWorldRenderPass& worldRenderPass,
	const Transform& lastWorldTransform,
	const Transform& worldTransform,
	render::Buffer* lastSkinBuffer,
	render::Buffer* skinBuffer,
	float distance,
	const IMeshParameterCallback* parameterCallback
) const
{
	auto it = m_parts.find(worldRenderPass.getTechnique());
	T_ASSERT(it != m_parts.end());

	// Setup the parameters; these are shared for all technique parts.
	auto programParams = renderContext->alloc< render::ProgramParameters >();
	programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(
		programParams,
		lastWorldTransform,
		worldTransform
	);

	if (parameterCallback)
		parameterCallback->setParameters(programParams);

	programParams->setBufferViewParameter(s_handleSkinBufferLast, lastSkinBuffer->getBufferView());
	programParams->setBufferViewParameter(s_handleSkinBuffer, skinBuffer->getBufferView());

	programParams->endParameters(renderContext);

	// Draw each technique part.
	const auto& meshParts = m_mesh->getParts();
	for (const auto& part : it->second)
	{
		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		render::SimpleRenderBlock* renderBlock = renderContext->allocNamed< render::SimpleRenderBlock >(L"SkinnedMesh");
		renderBlock->distance = distance;
		renderBlock->program = sp.program;
		renderBlock->programParams = programParams;
		renderBlock->indexBuffer = m_mesh->getIndexBuffer()->getBufferView();
		renderBlock->indexType = m_mesh->getIndexType();
		renderBlock->vertexBuffer = (m_mesh->getVertexBuffer() != nullptr) ? m_mesh->getVertexBuffer()->getBufferView() : nullptr;
		renderBlock->vertexLayout = m_mesh->getVertexLayout();
		renderBlock->primitives = meshParts[part.meshPart].primitives;

		renderContext->draw(
			sp.priority,
			renderBlock
		);
	}
}

int32_t SkinnedMesh::getJointCount() const
{
	return m_jointCount;
}

const SmallMap< std::wstring, int >& SkinnedMesh::getJointMap() const
{
	return m_jointMap;
}

Ref< render::Buffer > SkinnedMesh::createSkinBuffer(render::IRenderSystem* renderSystem) const
{
	const uint32_t vertexCount = m_mesh->getAuxBuffer(c_fccSkinPosition)->getBufferSize() / (6 * 4 * sizeof(float));
	return renderSystem->createBuffer(render::BuStructured, vertexCount * 6 * 4 * sizeof(float), false);
}

Ref< render::Buffer > SkinnedMesh::createJointBuffer(render::IRenderSystem* renderSystem, uint32_t jointCount)
{
	jointCount = std::max< uint32_t >(jointCount, 1);

	Ref< render::Buffer > jointBuffer = renderSystem->createBuffer(render::BuStructured, jointCount * sizeof(JointData), true);
	if (!jointBuffer)
		return nullptr;

	JointData* jointData = (JointData*)jointBuffer->lock();
	for (uint32_t i = 0; i < jointCount; ++i)
	{
		Vector4::zero().storeAligned(jointData->translation);
		Quaternion::identity().e.storeAligned(jointData->rotation);
		jointData++;
	}
	jointBuffer->unlock();

	return jointBuffer;
}

Ref< render::IAccelerationStructure > SkinnedMesh::createAccelerationStructure(render::IRenderSystem* renderSystem) const
{
	if (!renderSystem->supportRayTracing())
		return nullptr;

	const auto& part = m_mesh->getParts().back();
	T_FATAL_ASSERT(part.name == L"__RT__");

	AlignedVector< render::Primitives > primitives;
	primitives.push_back(part.primitives);

	return renderSystem->createAccelerationStructure(
		m_mesh->getAuxBuffer(SkinnedMesh::c_fccSkinPosition),
		m_rtVertexLayout,
		m_mesh->getIndexBuffer(),
		m_mesh->getIndexType(),
		primitives
	);
}

const render::Buffer* SkinnedMesh::getRTTriangleAttributes() const
{
	return m_mesh->getAuxBuffer(c_fccRayTracingTriangleAttributes);
}

const render::Buffer* SkinnedMesh::getRTVertexAttributes() const
{
	return m_mesh->getAuxBuffer(c_fccRayTracingVertexAttributes);
}

}
