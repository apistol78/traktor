#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix44.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;
class PrimitiveRenderer;

class T_DLLCLASS RenderControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	RenderControl(uint32_t matrixCount);

	bool create(ui::Widget* parent, IRenderSystem* renderSystem, db::Database* database);

	virtual void destroy();

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< IRenderView > m_renderView;
	Ref< PrimitiveRenderer > m_primitiveRenderer;
	ui::Point m_lastPoint;
	AlignedVector< Vector4 > m_angles;
	AlignedVector< Matrix44 > m_matrices;
	uint32_t m_editMatrix;

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventSize(ui::SizeEvent* event);
};

	}
}

