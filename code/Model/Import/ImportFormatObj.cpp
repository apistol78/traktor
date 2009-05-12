#include "Model/Import/ImportFormatObj.h"
#include "Model/Model.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Stream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/AnsiEncoding.h"
#include "Core/Misc/StringUtils.h"
#include "Core/Misc/SplitString.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.model.ImportFormatObj", ImportFormatObj, ImportFormat)

void ImportFormatObj::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Wavefront Object";
	outExtensions.push_back(L"obj");
}

bool ImportFormatObj::supportFormat(const Path& filePath) const
{
	return compareIgnoreCase(filePath.getExtension(), L"obj") == 0;
}

model::Model* ImportFormatObj::import(const Path& filePath, uint32_t importFlags) const
{
	Ref< Stream > stream = FileSystem::getInstance().open(filePath, File::FmRead);
	if (!stream)
		return 0;

	StringReader sr(stream, gc_new< AnsiEncoding >());
	std::wstring str;

	Ref< Model > md = gc_new< Model >();
	uint32_t materialId = c_InvalidIndex;

	while (sr.readLine(str) >= 0)
	{
		str = trim(str);
		if (str.empty() || str[0] == L'#')
			continue;

		if (startsWith(str, L"v "))
		{
			std::vector< float > values;
			if (Split< std::wstring, float >::any(str.substr(2), L" \t", values) >= 3)
			{
				md->addPosition(Vector4(
					values[0],
					values[1],
					values[2],
					1.0f
				));
			}
		}
		else if (startsWith(str, L"vt "))
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
		else if (startsWith(str, L"vn "))
		{
			std::vector< float > values;
			if (Split< std::wstring, float >::any(str.substr(2), L" \t", values) >= 3)
			{
				md->addNormal(Vector4(
					values[0],
					values[1],
					values[2],
					0.0f
				));
			}
		}
		else if (startsWith(str, L"f "))
		{
			std::vector< std::wstring > values;
			if (Split< std::wstring >::any(str.substr(2), L" \t", values) > 0)
			{
				Polygon polygon;
				polygon.setMaterial(materialId);

				for (std::vector< std::wstring >::const_iterator i = values.begin(); i != values.end(); ++i)
				{
					std::vector< int32_t > indices;
					if (Split< std::wstring, int32_t >::any(*i, L"/", indices) >= 0)
					{
						Vertex vertex;
						vertex.setPosition(indices[0] - 1);
						if (indices.size() >= 1 && indices[1] > 0)
							vertex.setTexCoord(indices[1] - 1);
						if (indices.size() >= 2 && indices[2] > 0)
							vertex.setNormal(indices[2] - 1);
						polygon.addVertex(md->addUniqueVertex(vertex));
					}
				}

				polygon.flipWinding();
				md->addUniquePolygon(polygon);
			}
		}
		else if (startsWith(str, L"usemtl "))
		{
			Material material;
			material.setName(str.substr(7));
			materialId = md->addMaterial(material);
		}
	}

	stream->close();
	return md;
}

	}
}
