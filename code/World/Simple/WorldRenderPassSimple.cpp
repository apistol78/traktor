#include "Render/Shader.h"
#include "Render/Context/ProgramParameters.h"
#include "World/WorldHandles.h"
#include "World/Simple/WorldRenderPassSimple.h"

namespace traktor
{
	namespace world
	{

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
}

render::handle_t WorldRenderPassSimple::getTechnique() const
{
	return m_technique;
}

uint32_t WorldRenderPassSimple::getPassFlags() const
{
	return PfFirst;
}

render::Shader::Permutation WorldRenderPassSimple::getPermutation(const render::Shader* shader) const
{
	return render::Shader::Permutation(m_technique);
}

void WorldRenderPassSimple::setProgramParameters(render::ProgramParameters* programParams) const
{
	const Matrix44 w = Matrix44::identity();
	programParams->attachParameters(m_globalProgramParams);
	programParams->setMatrixParameter(s_handleView, m_view);
	programParams->setMatrixParameter(s_handleViewInverse, m_viewInverse);
	programParams->setMatrixParameter(s_handleWorld, w);
	programParams->setMatrixParameter(s_handleWorldView, m_view * w);

}

void WorldRenderPassSimple::setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const
{
	const Matrix44 w = world.toMatrix44();
	programParams->attachParameters(m_globalProgramParams);
	programParams->setMatrixParameter(s_handleView, m_view);
	programParams->setMatrixParameter(s_handleViewInverse, m_viewInverse);
	programParams->setMatrixParameter(s_handleWorld, w);
	programParams->setMatrixParameter(s_handleWorldView, m_view * w);
}

	}
}
