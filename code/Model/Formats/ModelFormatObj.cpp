#include "Model/Formats/ModelFormatObj.h"
#include "Model/Model.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/AnsiEncoding.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatObj", 0, ModelFormatObj, ModelFormat)

void ModelFormatObj::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Wavefront Object";
	outExtensions.push_back(L"obj");
}

bool ModelFormatObj::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase< std::wstring >(extension, L"obj") == 0;
}

Ref< Model > ModelFormatObj::read(IStream* stream, uint32_t importFlags) const
{
	BufferedStream bs(stream);
	StringReader sr(&bs, new AnsiEncoding());
	std::wstring str;

	Ref< Model > md = new Model();
	uint32_t materialId = c_InvalidIndex;

	while (sr.readLine(str) >= 0)
	{
		str = trim(str);
		if (str.empty() || str[0] == L'#')
			continue;

		if ((importFlags & IfMeshPositions) != 0 && startsWith< std::wstring >(str, L"v "))
		{
			std::vector< float > values;
			if (Split< std::wstring, float >::any(str.substr(2), L" \t", values) >= 3)
			{
				md->addPosition(Vector4(
					-values[0],
					values[1],
					values[2],
					1.0f
				));
			}
		}
		else if ((importFlags & IfMeshPositions) != 0 && startsWith< std::wstring >(str, L"vt "))
		{
			std::vector< float > values;
			if (Split< std::wstring, float >::any(str.substr(2), L" \t", values) >= 2)
			{
				md->addTexCoord(Vector2(
					values[0],
					1.0f - values[1]
				));
			}
		}
		else if ((importFlags & IfMeshPositions) != 0 && startsWith< std::wstring >(str, L"vn "))
		{
			std::vector< float > values;
			if (Split< std::wstring, float >::any(str.substr(2), L" \t", values) >= 3)
			{
				md->addNormal(Vector4(
					-values[0],
					values[1],
					values[2],
					0.0f
				));
			}
		}
		else if ((importFlags & IfMeshPolygons) != 0 && startsWith< std::wstring >(str, L"f "))
		{
			std::vector< std::wstring > values;
			if (Split< std::wstring >::any(str.substr(2), L" \t", values) > 0)
			{
				Polygon polygon;
				polygon.setMaterial(materialId);

				for (std::vector< std::wstring >::const_iterator i = values.begin(); i != values.end(); ++i)
				{
					std::vector< int32_t > indices;
					if (Split< std::wstring, int32_t >::any(*i, L"/", indices, true) > 0)
					{
						Vertex vertex;
						vertex.setPosition(indices[0] - 1);
						if (indices.size() > 1 && indices[1] > 0)
							vertex.setTexCoord(0, indices[1] - 1);
						if (indices.size() > 2 && indices[2] > 0)
							vertex.setNormal(indices[2] - 1);
						polygon.addVertex(md->addUniqueVertex(vertex));
					}
				}

				md->addUniquePolygon(polygon);
			}
		}
		else if ((importFlags & IfMaterials) != 0 && (startsWith< std::wstring >(str, L"usemtl ") || startsWith< std::wstring >(str, L"g ")))
		{
			materialId = ~0UL;

			std::wstring materialName = str.substr(7);
			const std::vector< Material >& materials = md->getMaterials();
			for (uint32_t i = 0; i < uint32_t(materials.size()); ++i)
			{
				if (materials[i].getName() == materialName)
				{
					materialId = i;
					break;
				}
			}

			if (materialId == ~0UL)
			{
				Material material;
				material.setName(str.substr(7));
				materialId = md->addMaterial(material);
			}
		}
	}

	stream->close();
	return md;
}

bool ModelFormatObj::write(IStream* stream, const Model* model) const
{
	BufferedStream bs(stream, 512 * 1024);
	FileOutputStream s(&bs, new AnsiEncoding());

	s << L"o unnamed.obj" << Endl;
	s << Endl;
	s << L"#	             Vertex list" << Endl;
	s << Endl;

	const AlignedVector< Vector4 >& positions = model->getPositions();
	for (AlignedVector< Vector4 >::const_iterator i = positions.begin(); i != positions.end(); ++i)
		s << L"v " << i->x() << L" " << i->y() << L" " << i->z() << Endl;

	const AlignedVector< Vector2 >& texCoords = model->getTexCoords();
	for (AlignedVector< Vector2 >::const_iterator i = texCoords.begin(); i != texCoords.end(); ++i)
		s << L"vt " << i->x << L" " << i->y << Endl;

	const AlignedVector< Vector4 >& normals = model->getNormals();
	for (AlignedVector< Vector4 >::const_iterator i = normals.begin(); i != normals.end(); ++i)
		s << L"vn " << i->x() << L" " << i->y() << L" " << i->z() << Endl;

	s << Endl;
	s << L"#	    Point/Line/Face list" << Endl;
	s << Endl;

	const std::vector< Material >& materials = model->getMaterials();
	if (!materials.empty())
	{
		const std::vector< Polygon >& polygons = model->getPolygons();
		for (uint32_t material = 0; material < uint32_t(materials.size()); ++material)
		{
			s << L"usemtl " << materials[material].getName() << Endl;
			for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
			{
				if (i->getMaterial() != material)
					continue;

				const std::vector< uint32_t >& vertices = i->getVertices();

				s << L"f";
				for (std::vector< uint32_t >::const_reverse_iterator j = vertices.rbegin(); j != vertices.rend(); ++j)
				{
					const Vertex& vertex = model->getVertex(*j);

					s << L" " << vertex.getPosition() + 1;
					if (vertex.getTexCoord(0) != c_InvalidIndex)
						s << L"/" << vertex.getTexCoord(0) + 1;
					else
						s << L"/";
					if (vertex.getNormal() != c_InvalidIndex)
						s << L"/" << vertex.getNormal() + 1;
					else
						s << L"/";
				}
				s << Endl;
			}
			s << Endl;
		}
	}
	else
	{
		const std::vector< Polygon >& polygons = model->getPolygons();
		for (std::vector< Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
		{
			const std::vector< uint32_t >& vertices = i->getVertices();

			s << L"f";
			for (std::vector< uint32_t >::const_reverse_iterator j = vertices.rbegin(); j != vertices.rend(); ++j)
			{
				const Vertex& vertex = model->getVertex(*j);

				s << L" " << vertex.getPosition() + 1;
				if (vertex.getTexCoord(0) != c_InvalidIndex)
					s << L"/" << vertex.getTexCoord(0) + 1;
				else
					s << L"/";
				if (vertex.getNormal() != c_InvalidIndex)
					s << L"/" << vertex.getNormal() + 1;
				else
					s << L"/";
			}
			s << Endl;
		}
		s << Endl;
	}

	s << L"#	             End of file" << Endl;

	s.close();
	return true;
}

	}
}
