#include <algorithm>
#include <limits>
#include "Model/Formats/ModelFormatCollada.h"
#include "Model/Model.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Misc/String.h"
#include "Core/Misc/Split.h"
#include "Core/Log/Log.h"
#include "Xml/Document.h"
#include "Xml/Element.h"
#include "Xml/Attribute.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

struct FloatData
{
	std::wstring id;
	std::vector< float > data;
};

struct Input
{
	std::wstring semantic;
	std::wstring source;
	uint32_t offset;
};

struct PolygonData
{
	std::vector< uint32_t > vertexCounts;
	std::vector< uint32_t > indicies; 
	std::vector< Input > inputs;
	std::wstring material;
};

typedef std::pair< std::wstring, std::wstring > material_ref_t;
typedef std::pair< const FloatData*, uint32_t > source_data_info_t;

template < typename ValueType >
inline void parseStringToArray(const std::wstring& text, std::vector< ValueType >& outValueArray, uint32_t count)
{
	Split< std::wstring, ValueType >::any(text, L" ", outValueArray, count);
}

inline bool isReference(const std::wstring& node, const std::wstring& reference)
{
	return (reference[0] == '#') ? (reference.substr(1) == node) : false;
}

inline std::wstring dereference(const std::wstring& reference)
{
	if (reference.empty())
		return L"";
	return reference[0] == L'#' ? reference.substr(1) : reference;
}

void fetchPolygonData(PolygonData& polygonData, xml::Element* polyList, bool isTriangle)
{
	polygonData.material = polyList->getAttribute(L"material", L"")->getValue();
	uint32_t polyCount = parseString< uint32_t >(polyList->getAttribute(L"count", L"0")->getValue());

	RefArray< xml::Element > inputs;
	polyList->get(L"input", inputs);

	if (isTriangle)
	{
		uint32_t vertexCount = uint32_t(inputs.size());
		polygonData.vertexCounts = std::vector< uint32_t >(polyCount, vertexCount);
	}
	else
	{
		parseStringToArray(
			polyList->getSingle(L"vcount")->getValue(),
			polygonData.vertexCounts,
			polyCount
		);
	}

	uint32_t totalVertexCount = 0;
	for (uint32_t i = 0; i < polyCount; ++i)
		totalVertexCount += polygonData.vertexCounts[i];

	parseStringToArray(
		polyList->getSingle(L"p")->getValue(),
		polygonData.indicies,
		totalVertexCount * inputs.size()
	);

	polygonData.inputs.resize(inputs.size());
	for (size_t j = 0; j < inputs.size(); ++j)
	{
		polygonData.inputs[j].semantic = inputs[j]->getAttribute(L"semantic")->getValue();
		polygonData.inputs[j].offset = parseString< uint32_t >(inputs[j]->getAttribute(L"offset")->getValue());
		polygonData.inputs[j].source = inputs[j]->getAttribute(L"source")->getValue();
	}
}

source_data_info_t findSourceData(
	const std::wstring& semantic, 
	const PolygonData& polygonData, 
	const std::vector< FloatData >& sourceData,
	const std::pair< std::wstring, std::wstring >& vertexTranslation
)
{
	std::wstring source;
	uint32_t offset = 0;

	for (uint32_t i = 0; i < polygonData.inputs.size(); ++i)
	{
		if (semantic == polygonData.inputs[i].semantic)
		{
			offset = polygonData.inputs[i].offset;
			source = polygonData.inputs[i].source;
			if (isReference(vertexTranslation.first, source))
				source = vertexTranslation.second;
			break;
		}
	}

	for (uint32_t i = 0; i < sourceData.size(); ++i)
	{
		if (isReference(sourceData[i].id, source))
			return source_data_info_t(&sourceData[i], offset);
	}

	return source_data_info_t(0, offset);
}

void createMesh(xml::Element* libraryGeometries, const RefArray< xml::Element >& instanceGeometries, const std::vector< material_ref_t >& materialRefs, Model* outModel)
{
	for (size_t i = 0; i < instanceGeometries.size(); ++i)
	{
		std::wstring geometryRef = instanceGeometries[i]->getAttribute(L"url", L"")->getValue();

		Ref< xml::Element > mesh = libraryGeometries->getSingle(L"geometry[@name=" + dereference(geometryRef) + L"]/mesh");
		if (!mesh)
			continue;

		// Fetch the vertex data.
		std::vector< FloatData > vertexAttributeData;
		{
			RefArray< xml::Element > sources;
			mesh->get(L"source", sources);

			vertexAttributeData.resize(sources.size());
			for (uint32_t j = 0; j < sources.size(); ++j)
			{
				vertexAttributeData[j].id = sources[j]->getAttribute(L"id", L"")->getValue();

				Ref< const xml::Element > floatArray = sources[j]->getSingle(L"float_array");
				if (floatArray)
				{
					uint32_t floatCount = parseString< uint32_t >(floatArray->getAttribute(L"count", L"")->getValue());
					parseStringToArray(floatArray->getValue(), vertexAttributeData[j].data, floatCount);
				}
			}
		}

		Ref< xml::Element > vertices = mesh->getSingle(L"vertices");
		if (!vertices)
			continue;

		std::pair< std::wstring, std::wstring > vertexSourceTranslation;
		vertexSourceTranslation.first = vertices->getAttribute(L"name", L"")->getValue();
		vertexSourceTranslation.second = vertices->getSingle(L"input")->getAttribute(L"source", L"")->getValue();

		// Fetch polygon data.
		std::vector< PolygonData > polygonData;
		{
			RefArray< xml::Element > polyLists;
			mesh->get(L"polylist", polyLists);

			RefArray< xml::Element > triLists;
			mesh->get(L"triangles", triLists);

			polygonData.resize(polyLists.size() + triLists.size());
			
			uint32_t p = 0;
			for (uint32_t j = 0; j < polyLists.size(); ++j)
				fetchPolygonData(polygonData[p++], polyLists[j], false);

			for (uint32_t j = 0; j < triLists.size(); ++j)
				fetchPolygonData(polygonData[p++], triLists[j], true);
		}

		for (uint32_t j = 0; j < polygonData.size(); ++j)
		{
			uint32_t materialIndex = c_InvalidIndex;
			for (uint32_t k = 0; k < materialRefs.size(); ++k)
			{
				if (polygonData[j].material == materialRefs[k].first)
				{
					materialIndex = k;
					break;
				}
			}

			source_data_info_t vertexDataInfo = findSourceData(L"VERTEX", polygonData[j], vertexAttributeData, vertexSourceTranslation);
			source_data_info_t normalDataInfo = findSourceData(L"NORMAL", polygonData[j], vertexAttributeData, vertexSourceTranslation);
			source_data_info_t texcoordDataInfo = findSourceData(L"TEXCOORD", polygonData[j], vertexAttributeData, vertexSourceTranslation);

			uint32_t vertexOffset = vertexDataInfo.second;
			uint32_t normalOffset = normalDataInfo.second;
			uint32_t texcoordOffset = texcoordDataInfo.second;
			uint32_t indexOffset = 0;

			uint32_t vertexIndexStride = 0;
			if (vertexDataInfo.first)
				++vertexIndexStride;
			if (normalDataInfo.first)
				++vertexIndexStride;
			if (texcoordDataInfo.first)
				++vertexIndexStride;

			for (uint32_t k = 0; k < polygonData[j].vertexCounts.size(); ++k)
			{
				Polygon polygon;
				polygon.setMaterial(materialIndex);

				for (uint32_t l = 0; l < polygonData[j].vertexCounts[k]; ++l)
				{
					Vertex vertex;

					if (vertexDataInfo.first)
					{
						uint32_t positionIndex = polygonData[j].indicies[(indexOffset + l) * vertexIndexStride + vertexOffset];
						Vector4 position(
							vertexDataInfo.first->data[positionIndex * 3 + 0],
							vertexDataInfo.first->data[positionIndex * 3 + 1],
							vertexDataInfo.first->data[positionIndex * 3 + 2],
							1.0f
						);
						vertex.setPosition(outModel->addUniquePosition(position));
					}

					if (normalDataInfo.first)
					{
						uint32_t normalIndex = polygonData[j].indicies[(indexOffset + l) * vertexIndexStride + normalOffset];
						Vector4 normal(
							normalDataInfo.first->data[normalIndex * 3 + 0],
							normalDataInfo.first->data[normalIndex * 3 + 1],
							normalDataInfo.first->data[normalIndex * 3 + 2],
							0.0f
						);
						vertex.setNormal(outModel->addUniqueNormal(normal));
					}

					if (texcoordDataInfo.first)
					{
						uint32_t texCoordIndex = polygonData[j].indicies[(indexOffset + l) * vertexIndexStride + texcoordOffset];
						Vector2 texCoord(
							texcoordDataInfo.first->data[texCoordIndex * 2 + 0],
							1.0f - texcoordDataInfo.first->data[texCoordIndex * 2 + 1]
						);
						vertex.setTexCoord(outModel->addUniqueTexCoord(texCoord));
					}

					polygon.addVertex(
						outModel->addUniqueVertex(vertex)
					);
				}

				polygon.flipWinding();
				outModel->addPolygon(polygon);

				indexOffset += polygonData[j].vertexCounts[k];
			}
		}
	}
}

		}

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.model.ModelFormatCollada", ModelFormatCollada, ModelFormat)

void ModelFormatCollada::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Collada Object";
	outExtensions.push_back(L"dae");
}

bool ModelFormatCollada::supportFormat(const Path& filePath) const
{
	return 
		compareIgnoreCase(filePath.getExtension(), L"dae") == 0;
}

Model* ModelFormatCollada::read(const Path& filePath, uint32_t importFlags) const
{
	xml::Document doc;
	
	if (!doc.loadFromFile(filePath.getPathName()))
		return 0;

	Ref< xml::Element > scene = doc.getSingle(L"scene");
	if (!scene)
		return 0;

	Ref< xml::Element > instanceVisualScene = scene->getSingle(L"instance_visual_scene");
	const std::wstring& visualSceneRef = instanceVisualScene->getAttribute(L"url", L"")->getValue();

	Ref< xml::Element > visualScene = doc.getSingle(L"library_visual_scenes/visual_scene[@id=" + dereference(visualSceneRef) + L"]");
	if (!visualScene)
		return 0;

	// Find references to materials and geometries.
	std::vector< material_ref_t > materialRefs;
	RefArray< xml::Element > instanceGeometries;

	RefArray< xml::Element > nodes;
	visualScene->get(L"node", nodes);
	while (!nodes.empty())
	{
		Ref< xml::Element > node = nodes.back();
		nodes.pop_back();

		Ref< xml::Element > instanceGeometry = node->getSingle(L"instance_geometry");
		if (instanceGeometry)
		{
			instanceGeometries.push_back(instanceGeometry);

			RefArray< xml::Element > instanceMaterials;
			instanceGeometry->get(L"bind_material/technique_common/instance_material", instanceMaterials);

			for (RefArray< xml::Element >::iterator i = instanceMaterials.begin(); i != instanceMaterials.end(); ++i)
			{
				std::wstring symbol = (*i)->getAttribute(L"symbol", L"")->getValue();
				std::wstring target = (*i)->getAttribute(L"target", L"")->getValue();
				if (!symbol.empty() && !target.empty())
					materialRefs.push_back(material_ref_t(symbol, target));
			}
		}

		node->get(L"node", nodes);
	}

	// Create model
	Ref< Model > outModel = gc_new< Model >();

	if (importFlags & IfMaterials)
	{
		for (uint32_t i = 0; i < materialRefs.size(); ++i)
		{
			Material m;
			m.setName(materialRefs[i].first);
			m.setDoubleSided(false);
			outModel->addMaterial(m);
		}	
	}

	if (importFlags & IfMesh)
	{
		Ref< xml::Element > libraryGeometries = doc.getSingle(L"library_geometries");
		if (libraryGeometries)
			createMesh(libraryGeometries, instanceGeometries, materialRefs, outModel);
	}

	return outModel;
}

bool ModelFormatCollada::write(const Path& filePath, const Model* model) const
{
	return false;
}

	}
}
