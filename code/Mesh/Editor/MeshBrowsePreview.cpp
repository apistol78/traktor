#include "Core/Io/FileSystem.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Raster.h"
#include "Editor/IEditor.h"
#include "Mesh/Editor/MeshAsset.h"
#include "Mesh/Editor/MeshBrowsePreview.h"
#include "Model/Model.h"
#include "Model/ModelFormat.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MeshBrowsePreview", 0, MeshBrowsePreview, editor::IBrowsePreview)

TypeInfoSet MeshBrowsePreview::getPreviewTypes() const
{
	return makeTypeInfoSet< MeshAsset >();
}

Ref< ui::Bitmap > MeshBrowsePreview::generate(const editor::IEditor* editor, db::Instance* instance) const
{
	Ref< const MeshAsset > meshAsset = instance->getObject< MeshAsset >();
	if (!meshAsset)
		return 0;

	std::wstring assetPath = editor->getSettings()->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	Path fileName = FileSystem::getInstance().getAbsolutePath(assetPath, meshAsset->getFileName());

	Ref< model::Model > model = model::ModelFormat::readAny(fileName);
	if (!model)
		return 0;

	Aabb3 boundingBox = model->getBoundingBox();

	Ref< drawing::Image > meshThumb = new drawing::Image(
		drawing::PixelFormat::getR8G8B8A8(),
		ui::dpi96(64),
		ui::dpi96(64)
	);

	float cx = (float)(ui::dpi96(64) / 2.0f);
	float cy = (float)(ui::dpi96(64) / 2.0f);
	float hw = (float)(ui::dpi96(50) / 2.0f);
	float hh = (float)(ui::dpi96(50) / 2.0f);

	meshThumb->clear(Color4f(0.6f, 0.6f, 0.6f, 1.0f));

	drawing::Raster raster(meshThumb);

	const AlignedVector< model::Polygon >& polygons = model->getPolygons();
	const AlignedVector< model::Vertex >& vertices = model->getVertices();
	const AlignedVector< Vector4 >& positions = model->getPositions();
	const AlignedVector< Vector4 >& normals = model->getNormals();

	AlignedVector< Vector2 > screenVertices;

	for (AlignedVector< model::Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		const AlignedVector< uint32_t >& polygonVertices = i->getVertices();
		if (polygonVertices.empty())
			continue;

		raster.clear();

		for (size_t j = 0; j < polygonVertices.size(); ++j)
		{
			const model::Vertex& vertex = vertices[polygonVertices[j]];
			Vector4 position = positions[vertex.getPosition()];

			position = (position - boundingBox.getCenter()) / boundingBox.getExtent();

			float iz = 1.0f / (position.z() * 0.5f + 1.5f);

			if (j == 0)
			{
				raster.moveTo(
					position.x() * iz * hw + cx,
					cy - position.y() * iz * hh
				);
			}
			else
			{
				raster.lineTo(
					position.x() * iz * hw + cx,
					cy - position.y() * iz * hh
				);
			}
		}

		raster.close();

		float shade = 1.0f;
		if (i->getNormal() != model::c_InvalidIndex)
			shade = abs(normals[i->getNormal()].z() * 0.3f) + 0.7f;

		raster.clearStyles();
		raster.stroke(raster.defineSolidStyle(Color4f(shade, shade, shade, 1.0f)), 1.0f, drawing::Raster::ScRound);
	}

	raster.submit();

	return new ui::Bitmap(meshThumb);
}

	}
}
