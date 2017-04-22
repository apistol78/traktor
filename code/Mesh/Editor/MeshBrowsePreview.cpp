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
#include "Ui/Bitmap.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.MeshBrowsePreview", 0, MeshBrowsePreview, editor::IBrowsePreview)

TypeInfoSet MeshBrowsePreview::getPreviewTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshAsset >());
	return typeSet;
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
		64,
		64
	);

	meshThumb->clear(Color4f(0.6f, 0.6f, 0.6f, 1.0f));

	drawing::Raster raster(meshThumb);

	const std::vector< model::Polygon >& polygons = model->getPolygons();
	const std::vector< model::Vertex >& vertices = model->getVertices();
	const AlignedVector< Vector4 >& positions = model->getPositions();
	const AlignedVector< Vector4 >& normals = model->getNormals();

	AlignedVector< Vector2 > screenVertices;

	for (std::vector< model::Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	{
		const std::vector< uint32_t >& polygonVertices = i->getVertices();
		if (polygonVertices.empty())
			continue;

		raster.clear();

		for (uint32_t j = 0; j < polygonVertices.size(); ++j)
		{
			const model::Vertex& vertex = vertices[polygonVertices[j]];
			Vector4 position = positions[vertex.getPosition()];

			position = (position - boundingBox.getCenter()) / boundingBox.getExtent();

			float iz = 1.0f / (position.z() * 0.5f + 1.5f);

			if (j == 0)
			{
				raster.moveTo(
					position.x() * iz * 28.0f + 32.0f,
					32.0f - position.y() * iz * 28.0f
				);
			}
			else
			{
				raster.lineTo(
					position.x() * iz * 28.0f + 32.0f,
					32.0f - position.y() * iz * 28.0f
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
