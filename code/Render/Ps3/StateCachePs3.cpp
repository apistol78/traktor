#include "Render/Ps3/StateCachePs3.h"

namespace traktor
{
	namespace render
	{

StateCachePs3::StateCachePs3()
{
}

void StateCachePs3::setRenderState(const RenderState& rs)
{
	if (rs.cullFaceEnable != m_renderState.cullFaceEnable)
	{
		cellGcmSetCullFaceEnable(gCellGcmCurrentContext, rs.cullFaceEnable);
		m_renderState.cullFaceEnable = rs.cullFaceEnable;
	}

	if (rs.cullFace != m_renderState.cullFace)
	{
		cellGcmSetCullFace(gCellGcmCurrentContext, rs.cullFace);
		m_renderState.cullFace = rs.cullFace;
	}

	if (rs.blendEnable != m_renderState.blendEnable)
	{
		cellGcmSetBlendEnable(gCellGcmCurrentContext, rs.blendEnable);
		m_renderState.blendEnable = rs.blendEnable;
	}

	if (rs.blendEquation != m_renderState.blendEquation)
	{
		cellGcmSetBlendEquation(gCellGcmCurrentContext, rs.blendEquation, CELL_GCM_FUNC_ADD);
		m_renderState.blendEquation = rs.blendEquation;
	}

	if (rs.blendFuncSrc != m_renderState.blendFuncSrc || rs.blendFuncDest != m_renderState.blendFuncDest)
	{
		cellGcmSetBlendFunc(gCellGcmCurrentContext, rs.blendFuncSrc, rs.blendFuncDest, CELL_GCM_ONE, CELL_GCM_ZERO);
		m_renderState.blendFuncSrc = rs.blendFuncSrc;
		m_renderState.blendFuncDest = rs.blendFuncDest;
	}

	if (rs.depthTestEnable != m_renderState.depthTestEnable)
	{
		cellGcmSetDepthTestEnable(gCellGcmCurrentContext, rs.depthTestEnable);
		m_renderState.depthTestEnable = rs.depthTestEnable;
	}

	if (rs.colorMask != m_renderState.colorMask)
	{
		cellGcmSetColorMask(gCellGcmCurrentContext, rs.colorMask);
		m_renderState.colorMask = rs.colorMask;
	}

	if (rs.depthMask != m_renderState.depthMask)
	{
		cellGcmSetDepthMask(gCellGcmCurrentContext, rs.depthMask);
		m_renderState.depthMask = rs.depthMask;
	}

	if (rs.depthFunc != m_renderState.depthFunc)
	{
		cellGcmSetDepthFunc(gCellGcmCurrentContext, rs.depthFunc);
		m_renderState.depthFunc = rs.depthFunc;
	}

	if (rs.alphaTestEnable != m_renderState.alphaTestEnable)
	{
		cellGcmSetAlphaTestEnable(gCellGcmCurrentContext, rs.alphaTestEnable);
		m_renderState.alphaTestEnable = rs.alphaTestEnable;
	}

	if (rs.alphaFunc != m_renderState.alphaFunc || m_renderState.alphaRef != m_renderState.alphaRef)
	{
		cellGcmSetAlphaFunc(gCellGcmCurrentContext, rs.alphaFunc, rs.alphaRef);
		m_renderState.alphaFunc = rs.alphaFunc;
		m_renderState.alphaRef = rs.alphaRef;
	}
}

void StateCachePs3::reset()
{
	m_renderState = RenderState();

	cellGcmSetCullFaceEnable(gCellGcmCurrentContext, m_renderState.cullFaceEnable);
	cellGcmSetCullFace(gCellGcmCurrentContext, m_renderState.cullFace);
	cellGcmSetBlendEnable(gCellGcmCurrentContext, m_renderState.blendEnable);
	cellGcmSetBlendEquation(gCellGcmCurrentContext, m_renderState.blendEquation, CELL_GCM_FUNC_ADD);
	cellGcmSetBlendFunc(gCellGcmCurrentContext, m_renderState.blendFuncSrc, m_renderState.blendFuncDest, CELL_GCM_ONE, CELL_GCM_ZERO);
	cellGcmSetDepthTestEnable(gCellGcmCurrentContext, m_renderState.depthTestEnable);
	cellGcmSetColorMask(gCellGcmCurrentContext, m_renderState.colorMask);
	cellGcmSetDepthMask(gCellGcmCurrentContext, m_renderState.depthMask);
	cellGcmSetDepthFunc(gCellGcmCurrentContext, m_renderState.depthFunc);
	cellGcmSetAlphaTestEnable(gCellGcmCurrentContext, m_renderState.alphaTestEnable);
	cellGcmSetAlphaFunc(gCellGcmCurrentContext, m_renderState.alphaFunc, m_renderState.alphaRef);
}

	}
}
