#include "Core/Io/FileSystem.h"
#include "Core/Math/MathUtils.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Raster.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderGraphBrowsePreview.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderGraphBrowsePreview", 0, ShaderGraphBrowsePreview, editor::IBrowsePreview)

TypeInfoSet ShaderGraphBrowsePreview::getPreviewTypes() const
{
	return makeTypeInfoSet< ShaderGraph >();
}

Ref< ui::Bitmap > ShaderGraphBrowsePreview::generate(const editor::IEditor* editor, db::Instance* instance) const
{
	Ref< const ShaderGraph > shaderGraph = instance->getObject< ShaderGraph >();
	if (!shaderGraph)
		return nullptr;

	int32_t w = ui::dpi96(64);
	int32_t h = ui::dpi96(64);
	int32_t m = ui::dpi96(4);
	float sw = (float)ui::dpi96(100) / 100.0f;
	float ns = (float)ui::dpi96(200) / 100.0f;

	Ref< drawing::Image > shaderGraphThumb = new drawing::Image(
		drawing::PixelFormat::getR8G8B8A8(),
		w,
		h
	);

	shaderGraphThumb->clear(Color4f(0.2f, 0.2f, 0.2f, 0.0f));

	drawing::Raster raster(shaderGraphThumb);

	int32_t clearStyle = raster.defineSolidStyle(Color4f(0.2f, 0.2f, 0.2f, 0.8f));
	int32_t edgeStyle = raster.defineSolidStyle(Color4f(0.0f, 0.0f, 0.0f, 1.0f));
	int32_t nodeStyle = raster.defineSolidStyle(Color4f(1.0f, 1.0f, 0.8f, 1.0f));

	raster.clear();
	raster.moveTo(m, m);
	raster.lineTo(m, h - m);
	raster.lineTo(w - m, h - m);
	raster.lineTo(w - m, m);
	raster.lineTo(m, m);
	raster.fill(clearStyle, clearStyle, drawing::Raster::FillRule::NonZero);

	const RefArray< Node >& nodes = shaderGraph->getNodes();
	const RefArray< Edge >& edges = shaderGraph->getEdges();

	ui::Rect bounds;
	for (auto i = nodes.begin(); i != nodes.end(); ++i)
	{
		const auto position = (*i)->getPosition();
		if (i != nodes.begin())
		{
			bounds.left = min(bounds.left, position.first);
			bounds.top = min(bounds.top, position.second);
			bounds.right = max(bounds.right, position.first);
			bounds.bottom = max(bounds.bottom, position.second);
		}
		else
		{
			bounds.left = bounds.right = position.first;
			bounds.top = bounds.bottom = position.second;
		}
	}

	bounds.left -= m;
	bounds.top -= m;
	bounds.right += m;
	bounds.bottom += m;

	for (auto edge : edges)
	{
		if (!edge->getSource() || !edge->getDestination())
			continue;

		const auto position1 = edge->getSource()->getNode()->getPosition();
		const auto position2 = edge->getDestination()->getNode()->getPosition();

		float x1 = float((position1.first - bounds.left) * w) / bounds.getSize().cx;
		float y1 = float((position1.second - bounds.top) * h) / bounds.getSize().cy;
		float x2 = float((position2.first - bounds.left) * w) / bounds.getSize().cx;
		float y2 = float((position2.second - bounds.top) * h) / bounds.getSize().cy;

		raster.clear();
		raster.moveTo(x1, y1);
		raster.lineTo(x2, y2);
		raster.stroke(edgeStyle, sw, drawing::Raster::StrokeCap::Round);
	}

	for (auto node : nodes)
	{
		const auto position = node->getPosition();

		float x = float((position.first - bounds.left) * w) / bounds.getSize().cx;
		float y = float((position.second - bounds.top) * h) / bounds.getSize().cy;

		raster.clear();
		raster.moveTo(x - ns, y - ns);
		raster.lineTo(x + ns, y - ns);
		raster.lineTo(x + ns, y + ns);
		raster.lineTo(x - ns, y + ns);
		raster.lineTo(x - ns, y - ns);
		raster.fill(nodeStyle, nodeStyle, drawing::Raster::FillRule::NonZero);
	}

	raster.submit();

	return new ui::Bitmap(shaderGraphThumb);
}

	}
}
