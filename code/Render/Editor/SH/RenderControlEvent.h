#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix44.h"
#include "Ui/Event.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderView;
class PrimitiveRenderer;

class T_DLLCLASS RenderControlEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	RenderControlEvent(
		ui::EventSubject* sender,
		IRenderView* renderView,
		PrimitiveRenderer* primitiveRenderer,
		const AlignedVector< Vector4 >& angles,
		const AlignedVector< Matrix44 >& matrices
	);

	IRenderView* getRenderView() { return m_renderView; }

	PrimitiveRenderer* getPrimitiveRenderer() { return m_primitiveRenderer; }

	const AlignedVector< Vector4 >& getAngles() const { return m_angles; }

	const AlignedVector< Matrix44 >& getMatrices() const { return m_matrices; }

private:
	Ref< IRenderView > m_renderView;
	Ref< PrimitiveRenderer > m_primitiveRenderer;
	const AlignedVector< Vector4 >& m_angles;
	const AlignedVector< Matrix44 >& m_matrices;
};

	}
}
