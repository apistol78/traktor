#include <algorithm>
#include "Core/Math/Const.h"
#include "Model/ContainerHelpers.h"
#include "Model/Model.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

/*! \brief Return true if a replacing vertex match or "exceed" an existing vertex. */
bool shouldReplace(const Vertex& existing, const Vertex& replaceWith)
{
	if (
		existing.getPosition() != c_InvalidIndex &&
		existing.getPosition() != replaceWith.getPosition()
	)
		return false;

	if (
		existing.getColor() != c_InvalidIndex &&
		existing.getColor() != replaceWith.getColor()
	)
		return false;

	if (
		existing.getNormal() != c_InvalidIndex &&
		existing.getNormal() != replaceWith.getNormal()
	)
		return false;

	if (
		existing.getTangent() != c_InvalidIndex &&
		existing.getTangent() != replaceWith.getTangent()
	)
		return false;

	if (
		existing.getBinormal() != c_InvalidIndex &&
		existing.getBinormal() != replaceWith.getBinormal()
	)
		return false;

	if (existing.getTexCoordCount() > replaceWith.getTexCoordCount())
		return false;

	for (uint32_t i = 0; i < existing.getTexCoordCount(); ++i)
	{
		if (existing.getTexCoord(i) != replaceWith.getTexCoord(i))
			return false;
	}

	if (existing.getJointInfluenceCount() != replaceWith.getJointInfluenceCount())
		return false;

	for (uint32_t i = 0; i < existing.getJointInfluenceCount(); ++i)
	{
		if (abs(existing.getJointInfluence(i) - replaceWith.getJointInfluence(i)) > FUZZY_EPSILON)
			return false;
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Model", Model, Object)

Model::Model()
:	m_positions(2.0f)
,	m_colors(0.1f)
,	m_normals(0.1f)
,	m_texCoords(0.1f)
{
}

void Model::clear(uint32_t clearFlags)
{
	if (clearFlags & CfMaterials)
		m_materials.resize(0);
	if (clearFlags & CfVertices)
		m_vertices.resize(0);
	if (clearFlags & CfPolygons)
		m_polygons.resize(0);
	if (clearFlags & CfPositions)
		m_positions.clear();
	if (clearFlags & CfColors)
		m_colors.clear();
	if (clearFlags & CfNormals)
		m_normals.clear();
	if (clearFlags & CfTexCoords)
		m_texCoords.clear();
	if (clearFlags & CfJoints)
		m_joints.resize(0);

	for (std::vector< Vertex >::iterator i = m_vertices.begin(); i != m_vertices.end(); ++i)
	{
		if (clearFlags & CfPositions)
			i->setPosition(c_InvalidIndex);
		if (clearFlags & CfColors)
			i->setColor(c_InvalidIndex);
		if (clearFlags & CfNormals)
		{
			i->setNormal(c_InvalidIndex);
			i->setTangent(c_InvalidIndex);
			i->setBinormal(c_InvalidIndex);
		}
		if (clearFlags & CfTexCoords)
			i->clearTexCoords();
		if (clearFlags & CfJoints)
			i->clearJointInfluences();
	}

	for (std::vector< Polygon >::iterator i = m_polygons.begin(); i != m_polygons.end(); ++i)
	{
		if (clearFlags & CfMaterials)
			i->setMaterial(c_InvalidIndex);
		if (clearFlags & CfNormals)
			i->setNormal(c_InvalidIndex);
		if (clearFlags & CfVertices)
			i->clearVertices();
	}
}

Aabb3 Model::getBoundingBox() const
{
	Aabb3 boundingBox;
	for (AlignedVector< Vector4 >::const_iterator i = m_positions.values().begin(); i != m_positions.values().end(); ++i)
		boundingBox.contain(*i);
	return boundingBox;
}

uint32_t Model::addMaterial(const Material& material)
{
	return addId(m_materials, material);
}

uint32_t Model::addUniqueMaterial(const Material& material)
{
	return addUniqueId(m_materials, material, MaterialPredicate());
}

const Material& Model::getMaterial(uint32_t index) const
{
	return m_materials[index];
}

void Model::setMaterials(const std::vector< Material >& materials)
{
	m_materials = materials;
}

const std::vector< Material >& Model::getMaterials() const
{
	return m_materials;
}

uint32_t Model::addVertex(const Vertex& vertex)
{
	return addId(m_vertices, vertex);
}

uint32_t Model::addUniqueVertex(const Vertex& vertex)
{
	for (uint32_t i = 0; i < m_vertices.size(); ++i)
	{
		if (shouldReplace(m_vertices[i], vertex))
		{
			m_vertices[i] = vertex;
			return i;
		}
	}
	return addId(m_vertices, vertex);
}

void Model::setVertex(uint32_t index, const Vertex& vertex)
{
	T_ASSERT (index < uint32_t(m_vertices.size()));
	m_vertices[index] = vertex;
}

const Vertex& Model::getVertex(uint32_t index) const
{
	return m_vertices[index];
}

uint32_t Model::getVertexCount() const
{
	return uint32_t(m_vertices.size());
}

void Model::setVertices(const std::vector< Vertex >& vertices)
{
	m_vertices = vertices;
}

const std::vector< Vertex >& Model::getVertices() const
{
	return m_vertices;
}

void Model::reservePolygons(uint32_t polygonCapacity)
{
	m_polygons.reserve(polygonCapacity);
}

uint32_t Model::addPolygon(const Polygon& polygon)
{
	return addId(m_polygons, polygon);
}

uint32_t Model::addUniquePolygon(const Polygon& polygon)
{
	return addUniqueId< std::vector< Polygon >, Polygon, DefaultPredicate< Polygon > >(m_polygons, polygon);
}

void Model::setPolygon(uint32_t index, const Polygon& polygon)
{
	T_ASSERT (index < uint32_t(m_polygons.size()));
	m_polygons[index] = polygon;
}

const Polygon& Model::getPolygon(uint32_t index) const
{
	return m_polygons[index];
}

uint32_t Model::getPolygonCount() const
{
	return uint32_t(m_polygons.size());
}

void Model::setPolygons(const std::vector< Polygon >& polygons)
{
	m_polygons = polygons;
}

const std::vector< Polygon >& Model::getPolygons() const
{
	return m_polygons;
}

void Model::reservePositions(uint32_t positionCapacity)
{
	m_positions.reserve(positionCapacity);
}

uint32_t Model::addPosition(const Vector4& position)
{
	return m_positions.add(position);
}

uint32_t Model::addUniquePosition(const Vector4& position)
{
	uint32_t id = m_positions.get(position, 0.01f);
	return id != m_positions.InvalidIndex ? id : m_positions.add(position);
}

const Vector4& Model::getPosition(uint32_t index) const
{
	return m_positions.get(index);
}

const Vector4& Model::getVertexPosition(uint32_t vertexIndex) const
{
	return getPosition(getVertex(vertexIndex).getPosition());
}

void Model::setPositions(const AlignedVector< Vector4 >& positions)
{
	m_positions.replace(positions);
}

const AlignedVector< Vector4 >& Model::getPositions() const
{
	return m_positions.values();
}

uint32_t Model::addColor(const Vector4& color)
{
	return m_colors.add(color);
}

uint32_t Model::addUniqueColor(const Vector4& color)
{
	uint32_t id = m_colors.get(color, 1.0f / (4.0f * 256.0f));
	return id != m_colors.InvalidIndex ? id : m_colors.add(color);
}

const Vector4& Model::getColor(uint32_t index) const
{
	return m_colors.get(index);
}

void Model::setColors(const AlignedVector< Vector4 >& colors)
{
	m_colors.replace(colors);
}

const AlignedVector< Vector4 >& Model::getColors() const
{
	return m_colors.values();
}

uint32_t Model::addNormal(const Vector4& normal)
{
	return m_normals.add(normal);
}

uint32_t Model::addUniqueNormal(const Vector4& normal)
{
	Vector4 quantizedNormal(
		int(normal.x() * 128.0f) / 128.0f,
		int(normal.y() * 128.0f) / 128.0f,
		int(normal.z() * 128.0f) / 128.0f,
		0.0f
	);
	uint32_t id = m_normals.get(quantizedNormal, 0.008f);
	return id != m_normals.InvalidIndex ? id : m_normals.add(quantizedNormal);
}

const Vector4& Model::getNormal(uint32_t index) const
{
	return m_normals.get(index);
}

void Model::setNormals(const AlignedVector< Vector4 >& normals)
{
	m_normals.replace(normals);
}

const AlignedVector< Vector4 >& Model::getNormals() const
{
	return m_normals.values();
}

uint32_t Model::addTexCoord(const Vector2& texCoord)
{
	return m_texCoords.add(texCoord);
}

uint32_t Model::addUniqueTexCoord(const Vector2& texCoord)
{
	uint32_t id = m_texCoords.get(texCoord, 1.0f / 2048.0f);
	return id != m_texCoords.InvalidIndex ? id : m_texCoords.add(texCoord);
}

const Vector2& Model::getTexCoord(uint32_t index) const
{
	return m_texCoords.get(index);
}

void Model::setTexCoords(const AlignedVector< Vector2 >& texCoords)
{
	m_texCoords.replace(texCoords);
}

const AlignedVector< Vector2 >& Model::getTexCoords() const
{
	return m_texCoords.values();
}

uint32_t Model::addJoint(const std::wstring& jointName)
{
	return addUniqueId< std::vector< std::wstring >, std::wstring, DefaultPredicate< std::wstring > >(m_joints, jointName);
}

uint32_t Model::getJointCount() const
{
	return int(m_joints.size());
}

const std::wstring& Model::getJoint(uint32_t jointIndex) const
{
	return m_joints[jointIndex];
}

uint32_t Model::findJointIndex(const std::wstring& jointName) const
{
	std::vector< std::wstring >::const_iterator i = std::find(m_joints.begin(), m_joints.end(), jointName);
	return i != m_joints.end() ? uint32_t(std::distance(m_joints.begin(), i)) : c_InvalidIndex;
}

uint32_t Model::addBlendTarget(const std::wstring& blendTargetName)
{
	std::vector< std::wstring >::iterator i = std::find(m_blendTargets.begin(), m_blendTargets.end(), blendTargetName);
	if (i != m_blendTargets.end())
		return uint32_t(std::distance(m_blendTargets.begin(), i));

	m_blendTargets.push_back(blendTargetName);
	uint32_t id = uint32_t(m_blendTargets.size() - 1);

	m_blendTargetPositions[id] = m_positions.values();
	return id;
}

uint32_t Model::getBlendTargetCount() const
{
	return uint32_t(m_blendTargets.size());
}

const std::wstring& Model::getBlendTarget(uint32_t blendTargetIndex)
{
	return m_blendTargets[blendTargetIndex];
}

void Model::setBlendTargetPosition(uint32_t blendTargetIndex, uint32_t positionIndex, const Vector4& position)
{
	AlignedVector< Vector4 >& positions = m_blendTargetPositions[blendTargetIndex];
	if (positionIndex >= positions.size())
		positions.resize(positionIndex + 1);
	positions[positionIndex] = position;
}

const Vector4& Model::getBlendTargetPosition(uint32_t blendTargetIndex, uint32_t positionIndex) const
{
	std::map< uint32_t, AlignedVector< Vector4 > >::const_iterator i = m_blendTargetPositions.find(blendTargetIndex);
	return i->second[positionIndex];
}

	}
}
