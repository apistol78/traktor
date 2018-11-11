/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Editor/SH/RenderControlEvent.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderControlEvent", RenderControlEvent, ui::Event)

RenderControlEvent::RenderControlEvent(
	ui::EventSubject* sender, 
	IRenderView* renderView,
	PrimitiveRenderer* primitiveRenderer,
	const AlignedVector< Vector4 >& angles,
	const AlignedVector< Matrix44 >& matrices
)
:	ui::Event(sender)
,	m_renderView(renderView)
,	m_primitiveRenderer(primitiveRenderer)
,	m_angles(angles)
,	m_matrices(matrices)
{
}

	}
}
