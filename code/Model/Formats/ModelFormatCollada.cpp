#include <algorithm>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Model/Model.h"
#include "Model/Formats/ModelFormatCollada.h"
#include "Xml/Attribute.h"
#include "Xml/Document.h"
#include "Xml/Element.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

template < typename ValueType >
inline void parseStringToArray(const std::wstring& text, std::vector< ValueType >& outValueArray)
{
	outValueArray.reserve(64);
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

class FloatData
{
public:
	std::wstring getId() const { return m_id; }

	uint32_t getStride() const { return m_stride; }

	uint32_t getCount() const { return m_count; }

	bool read(Ref< xml::Element > source)
	{
		Ref< const xml::Element > floatArray = source->getSingle(L"float_array");
		if (floatArray)
		{
			m_id = source->getAttribute(L"id", L"")->getValue();
			parseStringToArray(floatArray->getValue(), m_data);
			if (Ref< xml::Element > techniqueCommon = source->getSingle(L"technique_common"))
			{
				if (Ref< const xml::Element > accessor = techniqueCommon->getSingle(L"accessor"))
				{
					m_stride = parseString< int >(accessor->getAttribute(L"stride", L"")->getValue());
					m_count = parseString< int >(accessor->getAttribute(L"count", L"")->getValue());
				}
			}
			return true;
		}
		else
			return false;
	}

	Matrix44 getMatrix(uint32_t index) const
	{
		return Matrix44::identity();
	}

	float getDataWeight(uint32_t index) const
	{
		T_ASSERT((index + 1)* m_stride <= m_data.size());
		return m_data[index * m_stride + 0];
	}

	Vector4 getDataPosition(uint32_t index) const
	{
		T_ASSERT((index + 1)* m_stride <= m_data.size());
		return Vector4(
			-m_data[index * m_stride + 0],
			m_data[index * m_stride + 1],
			m_data[index * m_stride + 2],
			1.0f
		);
	}

	Vector4 getDataNormal(uint32_t index) const
	{
		T_ASSERT((index + 1) * m_stride <= m_data.size());
		return Vector4(
			-m_data[index * m_stride + 0],
			m_data[index * m_stride + 1],
			m_data[index * m_stride + 2],
			0.0f
		);
	}

	Vector2 getTexcoord(uint32_t index) const
	{
		T_ASSERT((index + 1) * m_stride <= m_data.size());
		return Vector2(
			m_data[index * m_stride + 0],
			1.0f - m_data[index * m_stride + 1]
		);
	}

	Vector4 getDataColor(uint32_t index) const
	{
		T_ASSERT((index + 1) * m_stride <= m_data.size());
		return Vector4(
			m_data[index * m_stride + 0],
			m_data[index * m_stride + 1],
			m_data[index * m_stride + 2],
			m_stride == 4 ? m_data[index * m_stride + 3] : 0
		);
	}

private:
	std::wstring m_id;
	std::vector< float > m_data;
	int m_stride;
	int m_count;
};

typedef std::pair< std::wstring, std::wstring > material_ref_t;
typedef std::pair< const FloatData*, uint32_t > source_data_info_t;

struct NameData
{
	bool read(Ref< xml::Element > source)
	{
		Ref< const xml::Element > nameArray = source->getSingle(L"Name_array");
		if (!nameArray)
			nameArray = source->getSingle(L"IDREF_array").ptr();
		if (nameArray)
		{
			id = source->getAttribute(L"id", L"")->getValue();
			parseStringToArray(nameArray->getValue(), data);
			return true;
		}
		else
			return false;
	}
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

class PolygonData
{
public:

	Vector4 getPositionAttr(uint32_t index) const { return m_vertexDataInfo.first->getDataPosition(index); }
	
	uint32_t getPositionAttrCount() const { return m_vertexDataInfo.first->getCount(); }

	uint32_t getPolygonCount() const { return m_vertexCounts.size(); }

	uint32_t getVertexCount(int polygonIndex) const { return m_vertexCounts[polygonIndex]; }

	std::wstring getMaterial() const { return m_material; }

	uint32_t getAttribueIndex(int vertexIndex ,int attributeOffset) const { return m_indicies[ vertexIndex + attributeOffset ]; }

	uint32_t getStride() const
	{
		uint32_t stride = 0;
		for (uint32_t i = 0; i < m_inputs.size(); ++i)
		{
			if (m_inputs[i].offset > stride)
				stride = m_inputs[i].offset;
		}
		return stride + 1;
	}
	bool getPosition(Vector4& position, int& pIndex, int vIndex) const
	{
		if (m_vertexDataInfo.first)
		{
			pIndex = getAttribueIndex(vIndex ,m_vertexDataInfo.second);
			if (pIndex != -1)
			{
				position = m_vertexDataInfo.first->getDataPosition(pIndex);
				return true;
			}
		}
		return false;
	}

	bool getNormal(Vector4& normal, int vIndex) const
	{
		if (m_normalDataInfo.first)
		{
			uint32_t index = getAttribueIndex(vIndex, m_normalDataInfo.second);
			if (index != -1)
			{
				normal = m_normalDataInfo.first->getDataNormal(index);
				return true;
			}
		}
		return false;
	}

	bool getBiNormal(Vector4& binormal, int vIndex) const
	{
		if (m_biNormalDataInfo.first)
		{
			uint32_t index = getAttribueIndex(vIndex, m_biNormalDataInfo.second);
			if (index != -1)
			{
				binormal = m_biNormalDataInfo.first->getDataNormal(index);
				return true;
			}
		}
		return false;
	}

	bool getTangent(Vector4& tangent, int vIndex) const
	{
		if (m_tangentDataInfo.first)
		{
			uint32_t index = getAttribueIndex(vIndex, m_tangentDataInfo.second);
			if (index!= -1)
			{
				tangent = m_tangentDataInfo.first->getDataNormal(index);
			}
		}
		return false;
	}

	bool getTexCoord0(Vector2& texCoord, int vIndex) const
	{
		if (m_texcoord0DataInfo.first)
		{
			uint32_t index = getAttribueIndex(vIndex, m_texcoord0DataInfo.second);
			if (index != -1)
			{
				texCoord = m_texcoord0DataInfo.first->getTexcoord(index);
				return true;
			}
		}
		return false;
	}

	bool getTexCoord1(Vector2& texCoord, int vIndex) const
	{
		if (m_texcoord1DataInfo.first)
		{
			uint32_t index = getAttribueIndex(vIndex, m_texcoord1DataInfo.second);
			if (index != -1)
			{
				texCoord = m_texcoord1DataInfo.first->getTexcoord(index);
				return true;
			}
		}
		return false;
	}

	bool getVcolor(Vector4& vcolor, int vIndex) const
	{
		if (m_vcolorDataInfo.first)
		{
			uint32_t index = getAttribueIndex(vIndex, m_vcolorDataInfo.second);
			if (index != -1)
			{
				vcolor = m_vcolorDataInfo.first->getDataColor(index);
				return true;
			}
		}
		return false;
	}

	void setSourceData(const std::vector< FloatData >& sourceData, const std::pair< std::wstring, std::wstring >& vertexTranslation)
	{
		m_vertexDataInfo = findSourceData(L"VERTEX", 0, sourceData, vertexTranslation);
		m_normalDataInfo = findSourceData(L"NORMAL", 0, sourceData, vertexTranslation);
		m_texcoord0DataInfo = findSourceData(L"TEXCOORD", 0, sourceData, vertexTranslation);
		m_texcoord1DataInfo = findSourceData(L"TEXCOORD", 1, sourceData, vertexTranslation);
		m_vcolorDataInfo = findSourceData(L"COLOR", 0, sourceData, vertexTranslation);
		m_biNormalDataInfo = findSourceData(L"BINORMAL", 0, sourceData, vertexTranslation);
		m_tangentDataInfo = findSourceData(L"TANGENT", 0, sourceData, vertexTranslation);
		m_texBinormalDataInfo = findSourceData(L"TEXBINORMAL", 0, sourceData, vertexTranslation);
		m_texTangentDataInfo = findSourceData(L"TEXTANGENT", 0, sourceData, vertexTranslation);
	}

	void copyTopolgyAndMaterial(PolygonData* other)
	{
		m_vertexCounts = other->m_vertexCounts;
		m_indicies = other->m_indicies;
		m_material = other->m_material;
	}

	void read(xml::Element* polyList)
	{
		m_material = polyList->getAttribute(L"material", L"")->getValue();
		uint32_t polyCount = parseString< uint32_t >(polyList->getAttribute(L"count", L"0")->getValue());

		RefArray< xml::Element > inputs;
		polyList->get(L"input", inputs);
		m_inputs.resize(inputs.size());
		uint32_t stride = 0;
		for (size_t j = 0; j < inputs.size(); ++j)
		{
			m_inputs[j].read(inputs[j]);
			stride = max(stride, m_inputs[j].offset + 1);
		}
		RefArray< xml::Element > polyIndexLists;
		polyList->get(L"p", polyIndexLists);

		if (polyList->getSingle(L"vcount"))
			parseStringToArray(polyList->getSingle(L"vcount")->getValue(), m_vertexCounts);
		else if (polyIndexLists.size() == 1)
			m_vertexCounts = std::vector< uint32_t >(polyCount, 3);
		else
		{
			polyCount = polyIndexLists.size(); // to make sure we don't crash if there are polygons with holes.
			m_vertexCounts = std::vector< uint32_t >(polyCount);
			uint32_t oldn = 0;
			for (size_t i = 0; i < polyCount; i++)
			{
				uint32_t n = Split< std::wstring, uint32_t >::any(polyIndexLists[i]->getValue(), L" ", m_indicies);
				n /= stride;
				m_vertexCounts[i] = n - oldn;
				oldn = n;
			}
		
			return;
		}
		parseStringToArray(polyList->getSingle(L"p")->getValue(), m_indicies);
		reEnumerateSets();
	}

private:
	source_data_info_t findSourceData(
		const std::wstring& semantic, 
		int set,
		const std::vector< FloatData >& sourceData,
		const std::pair< std::wstring, std::wstring >& vertexTranslation
		) const
	{
		std::wstring source;
		uint32_t offset = 0;

		for (uint32_t i = 0; i < m_inputs.size(); ++i)
		{
			if (semantic == m_inputs[i].semantic && 
				set == m_inputs[i].set)
			{
				offset = m_inputs[i].offset;
				source = m_inputs[i].source;
				if (isReference(vertexTranslation.first, source))
					source = vertexTranslation.second;
				break;
			}
		}

		for (uint32_t i = 0; i < sourceData.size(); ++i)
		{
			if (isReference(sourceData[i].getId(), source))
				return source_data_info_t(&sourceData[i], offset);
		}

		return source_data_info_t((const FloatData*)0, offset);
	}

private:
	struct SetInfo
	{
		SetInfo(): min(1000),max(0), count(0) {}
		uint32_t min;
		uint32_t max;
		uint32_t count;
	};

	void reEnumerateSets()
	{
		std::wstring source;
		uint32_t offset = 0;
		std::map< std::wstring, SetInfo > setsPerSource;

		for (uint32_t i = 0; i < m_inputs.size(); ++i)
		{
			SetInfo& setInfo = setsPerSource[m_inputs[i].semantic];
			setInfo.count++;
			setInfo.min = std::min(setInfo.min, m_inputs[i].set);
			setInfo.max = std::max(setInfo.max, m_inputs[i].set);
		}
		for (uint32_t i = 0; i < m_inputs.size(); ++i)
			m_inputs[i].set -= setsPerSource[m_inputs[i].semantic].min;
	}

private:
	std::vector< uint32_t > m_vertexCounts;
	std::vector< uint32_t > m_indicies; 
	std::vector< Input > m_inputs;
	std::wstring m_material;
	source_data_info_t m_vertexDataInfo;		
	source_data_info_t m_normalDataInfo;		
	source_data_info_t m_texcoord0DataInfo;		
	source_data_info_t m_texcoord1DataInfo;		
	source_data_info_t m_vcolorDataInfo;		
	source_data_info_t m_biNormalDataInfo;		
	source_data_info_t m_tangentDataInfo;		
	source_data_info_t m_texBinormalDataInfo;	
	source_data_info_t m_texTangentDataInfo;	
};

class MorphTargetData
{
public:
	void read(xml::Element* xmlData)
	{
		// read the inputs (refs to joints + inverse bind matricies)
		RefArray< xml::Element > inputs;
		xmlData->get(L"input", inputs);
		for (size_t j = 0; j < inputs.size(); ++j)
		{
			if (inputs[j]->getAttribute(L"semantic")->getValue() == L"MORPH_TARGET")
				m_morphTargets.read(inputs[j]);
			if (inputs[j]->getAttribute(L"semantic")->getValue() == L"MORPH_WEIGHT")
				m_morphWeights.read(inputs[j]);
		}
	}
	Input m_morphTargets;
	Input m_morphWeights;
};

class MorphData
{
public:
	std::wstring getMorphSource() const { return m_morphSource; }

	std::wstring getMorphTarget(size_t index) const { return m_morphTargets.data[index]; }

	size_t getMorphTargetCount() const { return m_morphTargets.data.size(); }

	void read(xml::Element* morph)
	{
		m_morphSource = morph->getAttribute(L"source", L"")->getValue();

		Ref< xml::Element > targets = morph->getSingle(L"targets");
		m_morphTargetData.read(targets);

		RefArray< xml::Element > sources;
		morph->get(L"source", sources);	

		for (uint32_t j = 0; j < sources.size(); ++j)
		{
			std::wstring sourceName = sources[j]->getAttribute(L"id", L"")->getValue();
			if (isReference(sourceName, m_morphTargetData.m_morphTargets.source))
				m_morphTargets.read(sources[j]);
		}
	}

private:
	std::wstring m_morphSource;
	NameData m_morphTargets;
	MorphTargetData m_morphTargetData;
};

class VertexWeightData
{
public:
	void read(xml::Element* xmlData)
	{
		uint32_t count = parseString< uint32_t >(xmlData->getAttribute(L"count", L"0")->getValue());

		// read joints-weights per vertex counts
		parseStringToArray(xmlData->getSingle(L"vcount")->getValue(), m_vertexCounts);

		uint32_t totalVertexCount = 0;
		for (uint32_t i = 0; i < count; ++i)
			totalVertexCount += m_vertexCounts[i];

		// read indicies to joint-weight pairs
		parseStringToArray(
			xmlData->getSingle(L"v")->getValue(),
			m_indicies);

		// read the inputs (refs to joints + weights)
		RefArray< xml::Element > inputs;
		xmlData->get(L"input", inputs);
		for (size_t j = 0; j < inputs.size(); ++j)
		{
			if (inputs[j]->getAttribute(L"semantic")->getValue() == L"WEIGHT")
				m_weightInput.read(inputs[j]);
			if (inputs[j]->getAttribute(L"semantic")->getValue() == L"JOINT")
				m_jointInput.read(inputs[j]);
		}
	}

	std::vector< uint32_t > m_vertexCounts;
	std::vector< uint32_t > m_indicies; 
	Input m_weightInput;
	Input m_jointInput;
};

class JointData
{
public:
	void read(xml::Element* xmlData)
	{
		// read the inputs (refs to joints + inverse bind matricies)
		RefArray< xml::Element > inputs;
		xmlData->get(L"input", inputs);
		for (size_t j = 0; j < inputs.size(); ++j)
		{
			if (inputs[j]->getAttribute(L"semantic")->getValue() == L"INV_BIND_MATRIX")
				m_invBindMatrixInput.read(inputs[j]);
			if (inputs[j]->getAttribute(L"semantic")->getValue() == L"JOINT")
				m_jointInput.read(inputs[j]);
		}
	}
	Input m_invBindMatrixInput;
	Input m_jointInput;
};

class SkinData
{
public:
	std::wstring getSkinSource() const { return m_skinSource; }

	const Matrix44& getBindShapeMatrix() const { return m_bindShapeMatrix; }

	void read(xml::Element* skin)
	{
		m_skinSource = skin->getAttribute(L"source", L"")->getValue();

		// read bind shape matrix
		m_bindShapeMatrix = Matrix44::identity();
		if (Ref< xml::Element > bindShapeMatrix = skin->getSingle(L"bind_shape_matrix"))
		{
			std::vector< float > floatArray;
			parseStringToArray(bindShapeMatrix->getValue(), floatArray);
			if (floatArray.size() == 16)
				m_bindShapeMatrix = Matrix44(
					floatArray[ 0], floatArray[ 1], floatArray[ 2], floatArray[ 3],
					floatArray[ 4], floatArray[ 5], floatArray[ 6], floatArray[ 7],
					floatArray[ 8], floatArray[ 9], floatArray[10], floatArray[11],
					floatArray[12], floatArray[13], floatArray[14], floatArray[15]);
		}

		// read all sources
		RefArray< xml::Element > sources;
		skin->get(L"source", sources);			

		// read indicies to vertex weights 
		Ref< xml::Element > weights = skin->getSingle(L"vertex_weights");
		m_vertexWeightData.read(weights);

		// read vertex weightmap
		for (uint32_t j = 0; j < sources.size(); ++j)
		{
			std::wstring sourceName = sources[j]->getAttribute(L"id", L"")->getValue();
				if (isReference(sourceName, m_vertexWeightData.m_weightInput.source))
					m_weights.read(sources[j]);
				if (isReference(sourceName, m_vertexWeightData.m_jointInput.source))
					m_jointNames.read(sources[j]);
		}

		// read joints
		Ref< xml::Element > joints = skin->getSingle(L"joints");
		m_jointData.read(joints);

		// read inverse bind matricies
		for (uint32_t j = 0; j < sources.size(); ++j)
		{
			std::wstring sourceName = sources[j]->getAttribute(L"id", L"")->getValue();
			if (isReference(sourceName, m_jointData.m_invBindMatrixInput.source))
				m_invBindMatricies.read(sources[j]);
		}
	}

	void writeJointNames(Model* outModel)
	{
		int jointCount = m_jointNames.data.size();
		for (int i = 0; i < jointCount; i++)
			outModel->addJoint(m_jointNames.data[i]);
	}

	void setJointInfluence(Vertex& vertex, int index)
	{
		uint32_t offset = 0;
		uint32_t stride = 2;

		// Step forward to our vertex
		for (int i = 0; i < index; i++)
			offset += m_vertexWeightData.m_vertexCounts[i] * stride;

		// Get the number of joints that influence our vertex
		int jointCount = m_vertexWeightData.m_vertexCounts[index];

		// Get weights and indicies for all joints influencing our vertex
		for (int i = 0; i < jointCount; i++)
		{
			uint32_t jointIndex = m_vertexWeightData.m_indicies[offset + m_vertexWeightData.m_jointInput.offset];
			uint32_t weightIndex = m_vertexWeightData.m_indicies[offset + m_vertexWeightData.m_weightInput.offset];

			if (jointIndex != uint32_t(~0UL) && weightIndex < m_weights.getCount())
			{
				float weight = m_weights.getDataWeight(weightIndex);
				vertex.setJointInfluence(jointIndex, weight);
			}

			offset += stride;
		}
	}

private:
	FloatData m_weights;
	FloatData m_invBindMatricies;
	NameData m_jointNames;
	std::vector< Input > m_jointInputs;
	VertexWeightData m_vertexWeightData;
	JointData m_jointData;
	Matrix44 m_bindShapeMatrix;
	std::wstring m_skinSource;
};

class ColladaMeshData : public Object
{
	T_RTTI_CLASS

public:
	bool read(xml::Element* mesh, ColladaMeshData* morphBaseMesh)
	{
		{
			RefArray< xml::Element > sources;
			mesh->get(L"source", sources);

			m_vertexAttributeData.resize(sources.size());
			for (uint32_t j = 0; j < sources.size(); ++j)
			{
				m_vertexAttributeData[j].read(sources[j]);
			}
		}

		Ref< xml::Element > vertices = mesh->getSingle(L"vertices");
		if (!vertices)
			return false;

		m_vertexSourceTranslation.first = vertices->getAttribute(L"id", L"")->getValue();
		m_vertexSourceTranslation.second = vertices->getSingle(L"input")->getAttribute(L"source", L"")->getValue();

		// Fetch polygon data.
		{
			RefArray< xml::Element > polyLists;
			mesh->get(L"polylist", polyLists);

			RefArray< xml::Element > triLists;
			mesh->get(L"triangles", triLists);

			RefArray< xml::Element > polygons;
			mesh->get(L"polygons", polygons);

			m_polygonData.resize(polyLists.size() + triLists.size() + polygons.size());

			uint32_t p = 0;
			for (uint32_t j = 0; j < polyLists.size(); ++j)
				m_polygonData[p++].read(polyLists[j]);

			for (uint32_t j = 0; j < triLists.size(); ++j)
				m_polygonData[p++].read(triLists[j]);

			for (uint32_t j = 0; j < polygons.size(); ++j)
				m_polygonData[p++].read(polygons[j]);
		}
		if (morphBaseMesh)
		{	
			m_polygonData.resize(morphBaseMesh->m_polygonData.size());
			for (size_t i = 0; i < m_polygonData.size(); i++)
			{
				if (i)
					m_polygonData[i] = m_polygonData[0];
				m_polygonData[i].copyTopolgyAndMaterial(&(morphBaseMesh->m_polygonData[i]));
			}

		}
		for (uint32_t i = 0; i < m_polygonData.size(); ++i)
			m_polygonData[i].setSourceData(m_vertexAttributeData, m_vertexSourceTranslation);
		return true;
	}

	void addPositions(const Matrix44& transform, Model* outModel)
	{
		for (uint32_t j = 0; j < m_polygonData.size(); ++j)
		{
			uint32_t count = m_polygonData[j].getPositionAttrCount();
			for (uint32_t k = 0; k < count; k++)
			{
				Vector4 pos = transform * m_polygonData[j].getPositionAttr(k);
				uint32_t positionIndex = outModel->addPosition(pos);
				m_positionIndicies.push_back(positionIndex);
			}
		}
	}

	void addToModel(
		const Matrix44& transform,
		const std::map< std::wstring, uint32_t>& materialLookUp,
		SkinData* skinData,
		Model* outModel
	)
	{
		for (uint32_t j = 0; j < m_polygonData.size(); ++j)
		{
			uint32_t materialIndex = c_InvalidIndex;
			std::map<std::wstring, uint32_t>::const_iterator it = materialLookUp.find(m_polygonData[j].getMaterial());
			if (it != materialLookUp.end())
				materialIndex = it->second;

			uint32_t vertexIndexStride = m_polygonData[j].getStride();
			uint32_t indexOffset = 0;
			for (uint32_t k = 0; k < m_polygonData[j].getPolygonCount(); ++k)
			{
				Polygon polygon;
				polygon.setMaterial(materialIndex);
				for (uint32_t l = 0; l < m_polygonData[j].getVertexCount(k); ++l)
				{
					Vertex vertex;
					uint32_t vIndex = (indexOffset + l) * vertexIndexStride;
					Vector4 position;
					int pindex;
					if (m_polygonData[j].getPosition(position, pindex, vIndex))
					{
						vertex.setPosition(m_positionIndicies[pindex]);
						if (skinData)
							skinData->setJointInfluence(vertex, pindex);
					}
					Vector4 normal;
					if (m_polygonData[j].getNormal(normal, vIndex))
						vertex.setNormal(outModel->addUniqueNormal(normal));
					if (m_polygonData[j].getBiNormal(normal, vIndex))
						vertex.setBinormal(outModel->addUniqueNormal(normal));
					if (m_polygonData[j].getTangent(normal, vIndex))
						vertex.setTangent(outModel->addUniqueNormal(normal));
					Vector2 texCoord;
					if (m_polygonData[j].getTexCoord0(texCoord, vIndex))
						vertex.setTexCoord(0, outModel->addUniqueTexCoord(texCoord));
					if (m_polygonData[j].getTexCoord1(texCoord, vIndex))
						vertex.setTexCoord(1, outModel->addUniqueTexCoord(texCoord));
					Vector4 vcolor;
					if (m_polygonData[j].getVcolor(vcolor, vIndex))
						vertex.setColor(outModel->addUniqueColor(vcolor));
					polygon.addVertex(outModel->addUniqueVertex(vertex));
				}
				outModel->addPolygon(polygon);
				indexOffset += m_polygonData[j].getVertexCount(k);
			}
		}
	}

	void addMorphMeshToModel(
		const Matrix44& transform,
		const ColladaMeshData* morphMesh,
		uint32_t blendTargetIndex,
		Model* outModel
	)
	{
		uint32_t ix = 0;
		for (uint32_t j = 0; j < m_polygonData.size(); ++j)
		{
			uint32_t count = m_polygonData[j].getPositionAttrCount();
			for (uint32_t k = 0; k < count; k++)
			{
				Vector4 pos = transform * morphMesh->m_polygonData[j].getPositionAttr(k);
				outModel->setBlendTargetPosition(blendTargetIndex, m_positionIndicies[ix++], pos);
			}
		}
	}

private:
	std::vector< FloatData > m_vertexAttributeData;
	std::vector< PolygonData > m_polygonData;
	std::pair< std::wstring, std::wstring > m_vertexSourceTranslation;
	std::vector< uint32_t > m_positionIndicies; 
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ColladaMeshData", ColladaMeshData, Object)

void createMesh(
	xml::Element* libraryGeometries,
	xml::Element* libraryControllers,
	const RefArray< xml::Element >& instanceGeometryNodes,
	const RefArray< xml::Element >& instanceControllerNodes,
	const std::map< std::wstring, uint32_t >& materialRefs,
	Model* outModel
)
{
	for (size_t i = 0; i < instanceControllerNodes.size(); ++i)
	{
		Ref< xml::Element > instanceController = instanceControllerNodes[i]->getSingle(L"instance_controller");

		Matrix44 transform;
		if (Ref< xml::Element > scaleE = instanceControllerNodes[i]->getSingle(L"scale"))
		{
			std::vector< float > scaleArray;
			parseStringToArray(scaleE->getValue(), scaleArray);
			transform = scale(scaleArray[0],scaleArray[1], scaleArray[2]);
		}
		else
			transform = Matrix44::identity();

		std::wstring controllerRef = instanceController->getAttribute(L"url", L"")->getValue();

		Ref< xml::Element > skin = libraryControllers->getSingle(L"controller[@id=" + dereference(controllerRef) + L"]/skin");
		if (skin)
		{
			SkinData skinData;
			skinData.read(skin);
			skinData.writeJointNames(outModel);
			Ref< xml::Element > geometry = libraryGeometries->getSingle(L"geometry[@id=" + dereference(skinData.getSkinSource()) + L"]/mesh");
			if (!geometry)
				continue;
			ColladaMeshData meshData;
			if (meshData.read(geometry, 0))
			{
				Matrix44 bindShapeMatrix = skinData.getBindShapeMatrix();
				meshData.addPositions(bindShapeMatrix * transform, outModel);
				meshData.addToModel(bindShapeMatrix * transform, materialRefs, &skinData, outModel);
			}
			return;
		}

		MorphData morphData;
		Ref< xml::Element > morph = libraryControllers->getSingle(L"controller[@id=" + dereference(controllerRef) + L"]/morph");
		if (morph)
		{
			morphData.read(morph);
			ColladaMeshData baseMeshData;
			Ref< xml::Element > baseGeometry = libraryGeometries->getSingle(L"geometry[@id=" + dereference(morphData.getMorphSource()) + L"]/mesh");
			if (!baseGeometry)
				continue;
			if (baseMeshData.read(baseGeometry, 0))
				baseMeshData.addPositions(transform, outModel);
			for (size_t j = 0; j < morphData.getMorphTargetCount(); j++)
			{
				std::wstring morphTargetGeometry = morphData.getMorphTarget(j);
				Ref< xml::Element > morphGeometry = libraryGeometries->getSingle(L"geometry[@id=" + dereference(morphTargetGeometry) + L"]/mesh");
				ColladaMeshData meshData;
				if (meshData.read(morphGeometry, &baseMeshData))
				{
					uint32_t morphTargetIndex = outModel->addBlendTarget(morphTargetGeometry);
					baseMeshData.addMorphMeshToModel(transform, &meshData, morphTargetIndex, outModel);
				}
			}
			baseMeshData.addToModel(transform, materialRefs, 0, outModel);
			return;
		}
	}

	for (size_t i = 0; i < instanceGeometryNodes.size(); ++i)
	{
		Ref< xml::Element > instanceGeometry = instanceGeometryNodes[i]->getSingle(L"instance_geometry");

		std::wstring geometryRef = instanceGeometry->getAttribute(L"url", L"")->getValue();

		Ref< xml::Element > geometry = libraryGeometries->getSingle(L"geometry[@id=" + dereference(geometryRef) + L"]/mesh");
		if (!geometry)
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
		Matrix44 transform;
		if (Ref< xml::Element > scaleE = instanceGeometryNodes[i]->getSingle(L"scale"))
		{
			std::vector< float > scaleArray;
			parseStringToArray(scaleE->getValue(), scaleArray);
			transform = scale(scaleArray[0],scaleArray[1], scaleArray[2]);
		}
		else
			transform = Matrix44::identity();

		ColladaMeshData meshData;
		if (meshData.read(geometry, 0))
		{
			meshData.addPositions(transform, outModel);
			meshData.addToModel(transform, materialRefs, 0, outModel);
		}
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatCollada", 0, ModelFormatCollada, ModelFormat)

void ModelFormatCollada::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Collada Object";
	outExtensions.push_back(L"dae");
}

bool ModelFormatCollada::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase< std::wstring >(extension, L"dae") == 0;
}

Ref< Model > ModelFormatCollada::read(IStream* stream, uint32_t importFlags) const
{
	xml::Document doc;
	
	if (!doc.loadFromStream(stream))
		return 0;

	Ref< xml::Element > scene = doc.getSingle(L"scene");
	if (!scene)
		return 0;

	Ref< xml::Element > instanceVisualScene = scene->getSingle(L"instance_visual_scene");
	std::wstring visualSceneRef = instanceVisualScene->getAttribute(L"url", L"")->getValue();

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

		if (Ref< xml::Element > instanceGeometry = node->getSingle(L"instance_geometry"))
		{
			instanceGeometries.push_back(node);

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

		if (Ref< xml::Element > instanceController = node->getSingle(L"instance_controller"))
		{
			instanceControllers.push_back(node);

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
			m.setName(materialRefs[i].first);			// sg
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

bool ModelFormatCollada::write(IStream* stream, const Model* model) const
{
	return false;
}

	}
}
