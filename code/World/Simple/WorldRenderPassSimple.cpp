#include "Render/Shader.h"
#include "Render/Context/ProgramParameters.h"
#include "World/Simple/WorldRenderPassSimple.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

bool s_handlesInitialized = false;
render::handle_t s_techniqueSimpleColor;
render::handle_t s_handleView;
render::handle_t s_handleViewInverse;
render::handle_t s_handleWorld;
render::handle_t s_handleWorldView;

void initializeHandles()
{
	if (s_handlesInitialized)
		return;

	s_techniqueSimpleColor = render::getParameterHandle(L"World_SimpleColor");

	s_handleView = render::getParameterHandle(L"World_View");
	s_handleViewInverse = render::getParameterHandle(L"World_ViewInverse");
	s_handleWorld = render::getParameterHandle(L"World_World");
	s_handleWorldView = render::getParameterHandle(L"World_WorldView");

	s_handlesInitialized = true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldRenderPassSimple", WorldRenderPassSimple, IWorldRenderPass)

WorldRenderPassSimple::WorldRenderPassSimple(
	render::handle_t technique,
	render::ProgramParameters* globalProgramParams,
	const Matrix44& view
)
:	m_technique(technique)
,	m_globalProgramParams(globalProgramParams)
,	m_view(view)
,	m_viewInverse(view.inverse())
{
	initializeHandles();
}

render::handle_t WorldRenderPassSimple::getTechnique() const
{
	return m_technique;
}

uint32_t WorldRenderPassSimple::getPassFlags() const
{
	return PfFirst;
}

void WorldRenderPassSimple::setShaderTechnique(render::Shader* shader) const
{
	shader->setTechnique(m_technique);
}

void WorldRenderPassSimple::setShaderCombination(render::Shader* shader) const
{
}

void WorldRenderPassSimple::setProgramParameters(render::ProgramParameters* programParams) const
{
	Matrix44 w = Matrix44::identity();
	programParams->attachParameters(m_globalProgramParams);
	programParams->setMatrixParameter(s_handleView, m_view);
	programParams->setMatrixParameter(s_handleViewInverse, m_viewInverse);
	programParams->setMatrixParameter(s_handleWorld, w);
	programParams->setMatrixParameter(s_handleWorldView, m_view * w);

}

void WorldRenderPassSimple::setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world, const Aabb3& bounds) const
{
	Matrix44 w = world.toMatrix44();
	programParams->attachParameters(m_globalProgramParams);
	programParams->setMatrixParameter(s_handleView, m_view);
	programParams->setMatrixParameter(s_handleViewInverse, m_viewInverse);
	programParams->setMatrixParameter(s_handleWorld, w);
	programParams->setMatrixParameter(s_handleWorldView, m_view * w);
}

	}
}
