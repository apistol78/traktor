/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "Core/Math/MathUtils.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Raster.h"
#include "Render/Editor/Shader/ShaderGraphBrowsePreview.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"
#include "Render/Shader/ShaderGraph.h"
#include "Ui/Bitmap.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderGraphBrowsePreview", 0, ShaderGraphBrowsePreview, editor::IBrowsePreview)

TypeInfoSet ShaderGraphBrowsePreview::getPreviewTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ShaderGraph >());
	return typeSet;
}

Ref< ui::Bitmap > ShaderGraphBrowsePreview::generate(const editor::IEditor* editor, db::Instance* instance) const
{
	Ref< const ShaderGraph > shaderGraph = instance->getObject< ShaderGraph >();
	if (!shaderGraph)
		return 0;

	Ref< drawing::Image > shaderGraphThumb = new drawing::Image(
		drawing::PixelFormat::getR8G8B8A8(),
		64,
		64
	);

	shaderGraphThumb->clear(Color4f(1.0f, 1.0f, 1.0f, 0.0f));

	drawing::Raster raster(shaderGraphThumb);

	int32_t edgeStyle = raster.defineSolidStyle(Color4f(0.0f, 0.0f, 0.0f, 0.8f));
	int32_t nodeStyle = raster.defineSolidStyle(Color4f(1.0f, 1.0f, 0.8f, 1.0f));

	const RefArray< Node >& nodes = shaderGraph->getNodes();
	const RefArray< Edge >& edges = shaderGraph->getEdges();
	
	ui::Rect bounds;
	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		std::pair< int, int > position = (*i)->getPosition();
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

	bounds.left -= 4;
	bounds.top -= 4;
	bounds.right += 4;
	bounds.bottom += 4;
	
	for (RefArray< Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
	{
		std::pair< int, int > position1 = (*i)->getSource()->getNode()->getPosition();
		std::pair< int, int > position2 = (*i)->getDestination()->getNode()->getPosition();
		
		float x1 = float((position1.first - bounds.left) * 64) / bounds.getSize().cx;
		float y1 = float((position1.second - bounds.top) * 64) / bounds.getSize().cy;
		float x2 = float((position2.first - bounds.left) * 64) / bounds.getSize().cx;
		float y2 = float((position2.second - bounds.top) * 64) / bounds.getSize().cy;

		raster.clear();
		raster.moveTo(x1, y1);
		raster.lineTo(x2, y2);
		raster.stroke(edgeStyle, 1.0f, drawing::Raster::ScRound);
	}

	for (RefArray< Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
	{
		std::pair< int, int > position = (*i)->getPosition();
		
		float x = float((position.first - bounds.left) * 64) / bounds.getSize().cx;
		float y = float((position.second - bounds.top) * 64) / bounds.getSize().cy;
		
		raster.clear();
		raster.moveTo(x - 1.0f, y - 1.0f);
		raster.lineTo(x + 1.0f, y - 1.0f);
		raster.lineTo(x + 1.0f, y + 1.0f);
		raster.lineTo(x - 1.0f, y + 1.0f);
		raster.close();
		raster.fill(nodeStyle, nodeStyle, drawing::Raster::FrOddEven);
	}

	raster.submit();

	return new ui::Bitmap(shaderGraphThumb);
}

	}
}
