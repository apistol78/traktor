#include <algorithm>
#include <limits>
#include "Model/Formats/ModelFormatCollada.h"
#include "Model/Model.h"
#include "Core/Serialization/ISerializable.h"
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
	int stride;
};

struct NameData
{
	std::wstring id;
	std::vector< std::wstring > data;
};

struct Input
{
	void read(const xml::Element* xmlData)
	{
		semantic = xmlData->getAttribute(L"semantic")->getValue();
		source = xmlData->getAttribute(L"source")->getValue();
		if (xmlData->getAttribute(L"offset"))
			offset = parseString< uint32_t >(xmlData->getAttribute(L"offset")->getValue());
		else
			offset = 0;
		if (xmlData->getAttribute(L"set"))
			set = parseString< uint32_t >(xmlData->getAttribute(L"set")->getValue());
		else
			set = 0;
	}
	std::wstring semantic;
	std::wstring source;
	uint32_t offset;
	uint32_t set;
};

struct PolygonData
{
	std::vector< uint32_t > vertexCounts;
	std::vector< uint32_t > indicies; 
	std::vector< Input > inputs;
	std::wstring material;
};

struct VertexWeightData
{
	std::vector< uint32_t > vertexCounts;
	std::vector< uint32_t > indicies; 
	std::vector< Input > inputs;
};

typedef std::pair< std::wstring, std::wstring > material_ref_t;
typedef std::pair< const FloatData*, uint32_t > source_data_info_t;

template < typename ValueType >
inline void parseStringToArray(const std::wstring& text, std::vector< ValueType >& outValueArray)
{
	Split< std::wstring, ValueType >::any(text, L" \n", outValueArray);
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

uint32_t countSpace(const std::wstring& s)
{
	uint32_t n=0;
	for (uint32_t i=0; i < s.size(); i++)
		if (s[i] == L' ')
			n++;
	return n;
}

void fetchPolygonData(PolygonData& polygonData, xml::Element* polyList)
{
	polygonData.material = polyList->getAttribute(L"material", L"")->getValue();
	uint32_t polyCount = parseString< uint32_t >(polyList->getAttribute(L"count", L"0")->getValue());

	RefArray< xml::Element > inputs;
	polyList->get(L"input", inputs);
	polygonData.inputs.resize(inputs.size());
	uint32_t stride = 0;
	for (size_t j = 0; j < inputs.size(); ++j)
	{
		polygonData.inputs[j].read(inputs[j]);
		stride = max(stride, polygonData.inputs[j].offset + 1);
	}
	RefArray< xml::Element > polyIndexLists;
	polyList->get(L"p", polyIndexLists);

	if (polyList->getSingle(L"vcount"))
		parseStringToArray(
			polyList->getSingle(L"vcount")->getValue(),
			polygonData.vertexCounts);
	else if (polyIndexLists.size() == 1)
		polygonData.vertexCounts = std::vector< uint32_t >(polyCount, 3);
	else
	{
		polygonData.vertexCounts = std::vector< uint32_t >(polyCount);
		uint32_t oldn = 0;
		for (size_t i = 0; i < polyCount; i++)
		{
			uint32_t n = Split< std::wstring, uint32_t >::any(polyIndexLists[i]->getValue(), L" ", polygonData.indicies);
			n /= stride;
			polygonData.vertexCounts[i] = n - oldn;
			oldn = n;
		}
		return;
	}
	parseStringToArray(polyList->getSingle(L"p")->getValue(), polygonData.indicies);
}

void fetchVertexWeightData(VertexWeightData& vertexWeightData, xml::Element* xmlData)
{
	uint32_t count = parseString< uint32_t >(xmlData->getAttribute(L"count", L"0")->getValue());

	RefArray< xml::Element > inputs;
	xmlData->get(L"input", inputs);

	parseStringToArray(
		xmlData->getSingle(L"vcount")->getValue(),
		vertexWeightData.vertexCounts);

	uint32_t totalVertexCount = 0;
	for (uint32_t i = 0; i < count; ++i)
		totalVertexCount += vertexWeightData.vertexCounts[i];

	parseStringToArray(
		xmlData->getSingle(L"v")->getValue(),
		vertexWeightData.indicies);

	vertexWeightData.inputs.resize(inputs.size());
	for (size_t j = 0; j < inputs.size(); ++j)
		vertexWeightData.inputs[j].read(inputs[j]);
}

struct SetInfo
{
	SetInfo(): min(1000),max(0), count(0) {}
	uint32_t min;
	uint32_t max;
	uint32_t count;
};

void reEnumerateSets(PolygonData& polygonData)
{
	std::wstring source;
	uint32_t offset = 0;
	std::map< std::wstring, SetInfo > setsPerSource;

	for (uint32_t i = 0; i < polygonData.inputs.size(); ++i)
	{
		SetInfo& setInfo = setsPerSource[polygonData.inputs[i].semantic];
		setInfo.count++;
		setInfo.min = std::min(setInfo.min, polygonData.inputs[i].set);
		setInfo.max = std::max(setInfo.max, polygonData.inputs[i].set);
	}
	for (uint32_t i = 0; i < polygonData.inputs.size(); ++i)
		polygonData.inputs[i].set -= setsPerSource[polygonData.inputs[i].semantic].min;
}


source_data_info_t findSourceData(
	const std::wstring& semantic, 
	int set,
	const PolygonData& polygonData, 
	const std::vector< FloatData >& sourceData,
	const std::pair< std::wstring, std::wstring >& vertexTranslation
)
{
	std::wstring source;
	uint32_t offset = 0;

	for (uint32_t i = 0; i < polygonData.inputs.size(); ++i)
	{
		if (semantic == polygonData.inputs[i].semantic && 
			set == polygonData.inputs[i].set)
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


void createSkin(
				xml::Element* skin,
				Model* outModel
				)
{
	std::vector< FloatData > floatData;
	std::vector< NameData > nameData;
	{
		RefArray< xml::Element > sources;
		skin->get(L"source", sources);
			
		int nameArrayCount = 0;
		int floatArrayCount = 0;
		for (uint32_t j = 0; j < sources.size(); ++j)
		{
			Ref< const xml::Element > floatArray = sources[j]->getSingle(L"float_array");
			if (floatArray)
				floatArrayCount++;
			Ref< const xml::Element > nameArray = sources[j]->getSingle(L"Name_array");
			if (nameArray)
				nameArrayCount++;
		}

		floatData.resize(floatArrayCount);
		nameData.resize(nameArrayCount);
		nameArrayCount = 0;
		floatArrayCount = 0;
		for (uint32_t j = 0; j < sources.size(); ++j)
		{
			Ref< const xml::Element > floatArray = sources[j]->getSingle(L"float_array");
			if (floatArray)
			{
				floatData[floatArrayCount].id = sources[j]->getAttribute(L"id", L"")->getValue();
				parseStringToArray(floatArray->getValue(), floatData[floatArrayCount].data);
				floatArrayCount++;
			}
			Ref< const xml::Element > nameArray = sources[j]->getSingle(L"Name_array");
			if (nameArray)
			{
				nameData[nameArrayCount].id = sources[j]->getAttribute(L"id", L"")->getValue();
				parseStringToArray(nameArray->getValue(), nameData[nameArrayCount].data);
				nameArrayCount++;
			}
		}
		Ref< xml::Element > joints = skin->getSingle(L"joints");
		if (joints)
		{
			RefArray< xml::Element > inputs;
			joints->get(L"input", inputs);
			std::vector< Input > jointInputs;
			jointInputs.resize(inputs.size());
			for (uint32_t k = 0; k < inputs.size(); ++k)
			{
				jointInputs[k].read(inputs[k]);
//				outModel->
			}
		}

		Ref< xml::Element > weights = skin->getSingle(L"vertex_weights");
		VertexWeightData vertexWeightData;
		fetchVertexWeightData(vertexWeightData, weights);
	}
}

void createMesh(
	xml::Element* mesh,
	const std::map< std::wstring, uint32_t>& materialLookUp,
	Model* outModel
)
{
	std::vector< FloatData > vertexAttributeData;
	{
		RefArray< xml::Element > sources;
		mesh->get(L"source", sources);

		vertexAttributeData.resize(sources.size());
		for (uint32_t j = 0; j < sources.size(); ++j)
		{
			vertexAttributeData[j].id = sources[j]->getAttribute(L"id", L"")->getValue();
			vertexAttributeData[j].stride = -1;
			Ref< const xml::Element > floatArray = sources[j]->getSingle(L"float_array");
			if (floatArray)
			{
				parseStringToArray(floatArray->getValue(), vertexAttributeData[j].data);
				if (Ref< xml::Element > techniqueCommon = sources[j]->getSingle(L"technique_common"))
					if (Ref< const xml::Element > accessor = techniqueCommon->getSingle(L"accessor"))
					{
						vertexAttributeData[j].stride = parseString< int >(accessor->getAttribute(L"stride", L"")->getValue());
					}
			}
		}
	}

	Ref< xml::Element > vertices = mesh->getSingle(L"vertices");
	if (!vertices)
		return;

	std::pair< std::wstring, std::wstring > vertexSourceTranslation;
	vertexSourceTranslation.first = vertices->getAttribute(L"id", L"")->getValue();
	vertexSourceTranslation.second = vertices->getSingle(L"input")->getAttribute(L"source", L"")->getValue();

	// Fetch polygon data.
	std::vector< PolygonData > polygonData;
	{
		RefArray< xml::Element > polyLists;
		mesh->get(L"polylist", polyLists);

		RefArray< xml::Element > triLists;
		mesh->get(L"triangles", triLists);

		RefArray< xml::Element > polygons;
		mesh->get(L"polygons", polygons);

		polygonData.resize(polyLists.size() + triLists.size() + polygons.size());
		
		uint32_t p = 0;
		for (uint32_t j = 0; j < polyLists.size(); ++j)
			fetchPolygonData(polygonData[p++], polyLists[j]);

		for (uint32_t j = 0; j < triLists.size(); ++j)
			fetchPolygonData(polygonData[p++], triLists[j]);

		for (uint32_t j = 0; j < polygons.size(); ++j)
			fetchPolygonData(polygonData[p++], polygons[j]);
	}

	for (uint32_t j = 0; j < polygonData.size(); ++j)
	{
		uint32_t materialIndex = c_InvalidIndex;
		std::map<std::wstring, uint32_t>::const_iterator it = materialLookUp.find(polygonData[j].material);
		if (it != materialLookUp.end())
			materialIndex = it->second;
		reEnumerateSets(polygonData[j]);
		source_data_info_t vertexDataInfo = findSourceData(L"VERTEX", 0, polygonData[j], vertexAttributeData, vertexSourceTranslation);
		source_data_info_t normalDataInfo = findSourceData(L"NORMAL", 0, polygonData[j], vertexAttributeData, vertexSourceTranslation);
		source_data_info_t texcoord0DataInfo = findSourceData(L"TEXCOORD", 0, polygonData[j], vertexAttributeData, vertexSourceTranslation);
		source_data_info_t texcoord1DataInfo = findSourceData(L"TEXCOORD", 1, polygonData[j], vertexAttributeData, vertexSourceTranslation);
		source_data_info_t vcolorDataInfo = findSourceData(L"COLOR", 0, polygonData[j], vertexAttributeData, vertexSourceTranslation);
		source_data_info_t biNormalDataInfo = findSourceData(L"BINORMAL", 0, polygonData[j], vertexAttributeData, vertexSourceTranslation);
		source_data_info_t tangentDataInfo = findSourceData(L"TANGENT", 0, polygonData[j], vertexAttributeData, vertexSourceTranslation);
		source_data_info_t texBinormalDataInfo = findSourceData(L"TEXBINORMAL", 0, polygonData[j], vertexAttributeData, vertexSourceTranslation);
		source_data_info_t texTangentDataInfo = findSourceData(L"TEXTANGENT", 0, polygonData[j], vertexAttributeData, vertexSourceTranslation);

		uint32_t vertexOffset = vertexDataInfo.second;
		uint32_t normalOffset = normalDataInfo.second;
		uint32_t texcoord0Offset = texcoord0DataInfo.second;
		uint32_t texcoord1Offset = texcoord1DataInfo.second;
		uint32_t vcolorOffset = vcolorDataInfo.second;
		uint32_t biNormalOffset = biNormalDataInfo.second;
		uint32_t tangentOffset = tangentDataInfo.second;
		uint32_t texBinormalOffset = texBinormalDataInfo.second;
		uint32_t texTangentOffset = texTangentDataInfo.second;
		uint32_t indexOffset = 0;

		uint32_t vertexIndexStride = max(vertexOffset, normalOffset);
		vertexIndexStride = max(vertexIndexStride, texcoord0Offset);
		vertexIndexStride = max(vertexIndexStride, texcoord1Offset);
		vertexIndexStride = max(vertexIndexStride, vcolorOffset);
		vertexIndexStride = max(vertexIndexStride, biNormalOffset);
		vertexIndexStride = max(vertexIndexStride, tangentOffset);
		vertexIndexStride = max(vertexIndexStride, texBinormalOffset);
		vertexIndexStride = max(vertexIndexStride, texTangentOffset);
		vertexIndexStride += 1;

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
					if (positionIndex != -1)
					{
						T_ASSERT(positionIndex * 3 +3 <= vertexDataInfo.first->data.size());
						Vector4 position(
							-vertexDataInfo.first->data[positionIndex * 3 + 0],
							vertexDataInfo.first->data[positionIndex * 3 + 1],
							vertexDataInfo.first->data[positionIndex * 3 + 2],
							1.0f
						);
						vertex.setPosition(outModel->addUniquePosition(position));
					}
				}

				if (normalDataInfo.first)
				{
					uint32_t normalIndex = polygonData[j].indicies[(indexOffset + l) * vertexIndexStride + normalOffset];
					if (normalIndex != -1)
					{
						T_ASSERT(normalIndex * 3 +3 <= normalDataInfo.first->data.size());
						Vector4 normal(
							-normalDataInfo.first->data[normalIndex * 3 + 0],
							normalDataInfo.first->data[normalIndex * 3 + 1],
							normalDataInfo.first->data[normalIndex * 3 + 2],
							0.0f
						);
						vertex.setNormal(outModel->addUniqueNormal(normal));
					}
				}

				if (biNormalDataInfo.first)
				{
					uint32_t binormalIndex = polygonData[j].indicies[(indexOffset + l) * vertexIndexStride + biNormalOffset];
					if (binormalIndex != -1)
					{
						Vector4 binormal(
							-biNormalDataInfo.first->data[binormalIndex * 3 + 0],
							biNormalDataInfo.first->data[binormalIndex * 3 + 1],
							biNormalDataInfo.first->data[binormalIndex * 3 + 2],
							0.0f
							);
						vertex.setBinormal(outModel->addUniqueNormal(binormal));
					}
				}

				if (tangentDataInfo.first)
				{
					uint32_t tangentIndex = polygonData[j].indicies[(indexOffset + l) * vertexIndexStride + tangentOffset];
					if (tangentIndex!= -1)
					{
						Vector4 tangent(
							-tangentDataInfo.first->data[tangentIndex * 3 + 0],
							tangentDataInfo.first->data[tangentIndex * 3 + 1],
							tangentDataInfo.first->data[tangentIndex * 3 + 2],
							0.0f
							);
						vertex.setBinormal(outModel->addUniqueNormal(tangent));
					}
				}

				if (texcoord0DataInfo.first)
				{
					uint32_t texCoordIndex = polygonData[j].indicies[(indexOffset + l) * vertexIndexStride + texcoord0Offset];
					int stride = texcoord0DataInfo.first->stride;
					if (stride == -1)
						stride = 2;
					if (texCoordIndex != -1)
					{
						T_ASSERT(texCoordIndex * stride + stride <= texcoord0DataInfo.first->data.size());
						Vector2 texCoord(
									texcoord0DataInfo.first->data[texCoordIndex * stride + 0],
							 1.0f - texcoord0DataInfo.first->data[texCoordIndex * stride + 1]
						);
						vertex.setTexCoord(outModel->addUniqueTexCoord(texCoord));
					}
				}
				// Second uv set
				if (texcoord1DataInfo.first)
				{
					uint32_t texCoordIndex = polygonData[j].indicies[(indexOffset + l) * vertexIndexStride + texcoord1Offset];
					int stride = texcoord1DataInfo.first->stride;
					if (stride == -1)
						stride = 2;
					if (texCoordIndex != -1)
					{
						Vector2 texCoord(
									texcoord1DataInfo.first->data[texCoordIndex * stride + 0],
							 1.0f - texcoord1DataInfo.first->data[texCoordIndex * stride + 1]
						);
						// Unsupported!
						//vertex.setTexCoord(outModel->addUniqueTexCoord(texCoord));
					}
				}

				if (vcolorDataInfo.first)
				{
					uint32_t vcolorIndex = polygonData[j].indicies[(indexOffset + l) * vertexIndexStride + vcolorOffset];
					if (vcolorIndex != -1)
					{
						int stride = vcolorDataInfo.first->stride;
						if (stride == -1)
							stride = 4;
						T_ASSERT(vcolorIndex * stride <= vcolorDataInfo.first->data.size());						
						Vector4 vcolor(
							vcolorDataInfo.first->data[vcolorIndex * stride + 0],
							vcolorDataInfo.first->data[vcolorIndex * stride + 1],
							vcolorDataInfo.first->data[vcolorIndex * stride + 2],
							stride == 4 ? vcolorDataInfo.first->data[vcolorIndex * stride + 3] : 0
							);
						vertex.setColor(outModel->addColor(vcolor));
					}
				}

				polygon.addVertex(
					outModel->addUniqueVertex(vertex)
				);
			}

//			polygon.flipWinding();
			outModel->addPolygon(polygon);

			indexOffset += polygonData[j].vertexCounts[k];
		}
	}
}

void createMesh(
	xml::Element* libraryGeometries,
	xml::Element* libraryControllers,
	const RefArray< xml::Element >& instanceGeometries,
	const RefArray< xml::Element >& instanceControllers,
	const std::map< std::wstring, uint32_t >& materialRefs,
	Model* outModel
)
{
	for (size_t i = 0; i < instanceControllers.size(); ++i)
	{
		std::wstring controllerRef = instanceControllers[i]->getAttribute(L"url", L"")->getValue();

		Ref< xml::Element > skin = libraryControllers->getSingle(L"controller[@id=" + dereference(controllerRef) + L"]/skin");
		if (!skin)
			continue;
		createSkin(skin, outModel);

		std::wstring geometryRef = skin->getAttribute(L"source", L"")->getValue();

		Ref< xml::Element > mesh = libraryGeometries->getSingle(L"geometry[@id=" + dereference(geometryRef) + L"]/mesh");
		if (!mesh)
			continue;

		createMesh(mesh, materialRefs, outModel);
	}

	for (size_t i = 0; i < instanceGeometries.size(); ++i)
	{
		std::wstring geometryRef = instanceGeometries[i]->getAttribute(L"url", L"")->getValue();

		Ref< xml::Element > mesh = libraryGeometries->getSingle(L"geometry[@id=" + dereference(geometryRef) + L"]/mesh");
		if (!mesh)
			continue;
/*
		Ref< xml::Element > extra = libraryGeometries->getSingle(L"geometry[@id=" + dereference(geometryRef) + L"]/extra");
		bool doubleSided = false;
		if (extra)
		{
			Ref< xml::Element > technique = extra->getSingle(L"technique");
			if (technique)
				if (technique->getSingle(L"double_sided") && technique->getSingle(L"double_sided")->getValue() == L"1")
					doubleSided = true;

		}
*/
		createMesh(mesh, materialRefs, outModel);
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatCollada", 0, ModelFormatCollada, ModelFormat)

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
/*
#include "Core/Math/Const.h"
#include "Animation/Skeleton.h"
#include "Animation/Bone.h"
namespace {
Transform parseTranslation(xml::Element* node)
{
	if (node)
	{
		std::vector< float > t;
		parseStringToArray(node->getValue(), t);
		return Transform(Vector4(t[0], t[1], t[2], 0));
	}
	else 
		return Transform::identity();
}

Transform parseRotation(xml::Element* node)
{
	if (node)
	{
		std::vector< float > t;
		parseStringToArray(node->getValue(), t);
		float angle = deg2rad( t[3]);
		return Transform(Quaternion(Vector4(t[0] * angle, t[1] * angle, t[2] * angle, 0)));
	}
	else 
		return Transform::identity();
}

void parseJoint(xml::Element* node, animation::Skeleton& skeleton, int level)
{
	int parent = int(skeleton.getBoneCount()) - 1;
	if (node->getAttribute(L"type")->getValue() == L"JOINT")
	{
		Transform t;
		Ref<xml::Element> matrix = node->getSingle(L"matrix");
		if (matrix)
		{
			Matrix44 m;
			std::vector< float> ma;
			parseStringToArray(matrix->getValue(),ma);
			m = Matrix44(ma[0], ma[1], ma[2], ma[3], 
						 ma[4], ma[5], ma[6], ma[7], 
						 ma[8], ma[9], ma[10], ma[11], 
						 ma[12], ma[13], ma[14], ma[15]);
			t = Transform(m);
		}
		else
		{
			Transform transform = parseTranslation(node->getSingle(L"translate"));
			RefArray< xml::Element > rotations;
			node->get(L"rotate", rotations);
			for (uint32_t i=0; i < rotations.size(); i++)
			{
				Transform rotation = parseRotation(rotations[i]);
				t = t * rotation;
			}
		}
		animation::Bone b;
		if (level > 0)
			b.setParent(level - 1);
		b.setName(node->getAttribute(L"name")->getValue());
		b.setOrientation(t.rotation());
		level++;
	}
	else
	{
		return;
	}
}
}
*/

Ref< Model > ModelFormatCollada::read(const Path& filePath, uint32_t importFlags) const
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
	std::map< std::wstring, uint32_t> materialLookUp;
	RefArray< xml::Element > instanceGeometries;
	RefArray< xml::Element > instanceControllers;

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

		Ref< xml::Element > instanceController = node->getSingle(L"instance_controller");
		if (instanceController)
		{
			instanceControllers.push_back(instanceController);

			RefArray< xml::Element > instanceMaterials;
			instanceController->get(L"bind_material/technique_common/instance_material", instanceMaterials);

			for (RefArray< xml::Element >::iterator i = instanceMaterials.begin(); i != instanceMaterials.end(); ++i)
			{
				std::wstring symbol = (*i)->getAttribute(L"symbol", L"")->getValue();
				std::wstring target = (*i)->getAttribute(L"target", L"")->getValue();
				if (!symbol.empty() && !target.empty())
					materialRefs.push_back(material_ref_t(symbol, target));
			}
		}
		if (node->getAttribute(L"type") && node->getAttribute(L"type")->getValue() == L"JOINT")
		{
/*
			animation::Skeleton skeleton;
			parseJoint(node, skeleton, 0);
*/
		}

		node->get(L"node", nodes);
	}

	// Create model
	Ref< Model > outModel = new Model();

	for (uint32_t i = 0; i < materialRefs.size(); ++i)
	{
		std::map<std::wstring, uint32_t>::iterator it = materialLookUp.find(materialRefs[i].first);
		if (it == materialLookUp.end())
		{
			Material m;
			//m.setName(materialRefs[i].second);		// material
			m.setName(materialRefs[i].first);		// sg
			m.setDoubleSided(false);
			uint32_t materialIndex = outModel->addMaterial(m);
			materialLookUp.insert(std::make_pair(materialRefs[i].first, materialIndex));
		}
	}

	if (importFlags & IfMesh)
	{
		Ref< xml::Element > libraryGeometries = doc.getSingle(L"library_geometries");
		Ref< xml::Element > libraryControllers = doc.getSingle(L"library_controllers");

		createMesh(
			libraryGeometries,
			libraryControllers,
			instanceGeometries,
			instanceControllers,
			materialLookUp,
			outModel
		);
	}

	return outModel;
}

bool ModelFormatCollada::write(const Path& filePath, const Model* model) const
{
	return false;
}

	}
}
