#ifndef traktor_model_Model_H
#define traktor_model_Model_H

#include <map>
#include "Core/Object.h"
#include "Core/Math/Aabb3.h"
#include "Model/Types.h"
#include "Model/Grid2.h"
#include "Model/Grid3.h"
#include "Model/Material.h"
#include "Model/Polygon.h"
#include "Model/Vertex.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

/*! \brief Intermediate 3D model.
 * \ingroup Model
 */
class T_DLLCLASS Model : public Object
{
	T_RTTI_CLASS;

public:
	enum ClearFlags
	{
		CfMaterials	= (1 << 0),
		CfVertices = (1 << 1),
		CfPolygons = (1 << 2),
		CfPositions = (1 << 3),
		CfColors = (1 << 4),
		CfNormals = (1 << 5),
		CfTexCoords = (1 << 6),
		CfJoints = (1 << 7),
		CfAll = (CfMaterials | CfVertices | CfPolygons | CfPositions | CfNormals | CfTexCoords | CfJoints)
	};

	Model();

	void clear(uint32_t clearFlags = CfAll);

	Aabb3 getBoundingBox() const;

	uint32_t addMaterial(const Material& material);

	uint32_t addUniqueMaterial(const Material& material);

	const Material& getMaterial(uint32_t index) const { return m_materials[index]; }

	void setMaterials(const std::vector< Material >& materials) { m_materials = materials; }

	const std::vector< Material >& getMaterials() const { return m_materials; }

	void reserveVertices(uint32_t vertexCapacity) { m_vertices.reserve(vertexCapacity); }

	uint32_t addVertex(const Vertex& vertex);

	uint32_t addUniqueVertex(const Vertex& vertex);

	void setVertex(uint32_t index, const Vertex& vertex) { m_vertices[index] = vertex; }

	const Vertex& getVertex(uint32_t index) const { return m_vertices[index]; }

	uint32_t getVertexCount() const { return uint32_t(m_vertices.size()); }

	void setVertices(const std::vector< Vertex >& vertices) { m_vertices = vertices; }

	const std::vector< Vertex >& getVertices() const { return m_vertices; }

	void reservePolygons(uint32_t polygonCapacity) { m_polygons.reserve(polygonCapacity); }

	uint32_t addPolygon(const Polygon& polygon);

	uint32_t addUniquePolygon(const Polygon& polygon);

	void setPolygon(uint32_t index, const Polygon& polygon) { m_polygons[index] = polygon; }

	const Polygon& getPolygon(uint32_t index) const { return m_polygons[index]; }

	uint32_t getPolygonCount() const { return uint32_t(m_polygons.size()); }

	void setPolygons(const std::vector< Polygon >& polygons) { m_polygons = polygons; }

	const std::vector< Polygon >& getPolygons() const { return m_polygons; }

	std::vector< Polygon >& getPolygons() { return m_polygons; }

	void reservePositions(uint32_t positionCapacity);

	uint32_t addPosition(const Vector4& position);

	uint32_t addUniquePosition(const Vector4& position);

	uint32_t getPositionCount() const { return m_positions.size(); }

	const Vector4& getPosition(uint32_t index) const { return m_positions.get(index); }

	const Vector4& getVertexPosition(uint32_t vertexIndex) const { return getPosition(getVertex(vertexIndex).getPosition()); }

	void setPositions(const AlignedVector< Vector4 >& positions) { m_positions.replace(positions); }

	const AlignedVector< Vector4 >& getPositions() const { return m_positions.values(); }

	uint32_t addColor(const Vector4& color);

	uint32_t addUniqueColor(const Vector4& color);

	uint32_t getColorCount() const { return m_colors.size(); }

	const Vector4& getColor(uint32_t index) const { return m_colors.get(index); }

	void setColors(const AlignedVector< Vector4 >& colors) { m_colors.replace(colors); }

	const AlignedVector< Vector4 >& getColors() const { return m_colors.values(); }

	void reserveColors(uint32_t colorCapacity);

	uint32_t addNormal(const Vector4& normal);

	uint32_t addUniqueNormal(const Vector4& normal);

	uint32_t getNormalCount() const { return m_normals.size(); }

	const Vector4& getNormal(uint32_t index) const { return m_normals.get(index); }

	void setNormals(const AlignedVector< Vector4 >& normals) { m_normals.replace(normals); }

	const AlignedVector< Vector4 >& getNormals() const { return m_normals.values(); }

	void reserveNormals(uint32_t normalCapacity);

	uint32_t addTexCoord(const Vector2& texCoord);

	uint32_t addUniqueTexCoord(const Vector2& texCoord);

	const Vector2& getTexCoord(uint32_t index) const { return m_texCoords.get(index); }

	void setTexCoords(const AlignedVector< Vector2 >& texCoords) { m_texCoords.replace(texCoords); }

	const AlignedVector< Vector2 >& getTexCoords() const { return m_texCoords.values(); }

	uint32_t addJoint(const std::wstring& jointName);

	uint32_t getJointCount() const { return int(m_joints.size()); }

	const std::wstring& getJoint(uint32_t jointIndex) const { return m_joints[jointIndex]; }

	uint32_t findJointIndex(const std::wstring& jointName) const;

	uint32_t addBlendTarget(const std::wstring& blendTargetName);

	uint32_t getBlendTargetCount() const { return uint32_t(m_blendTargets.size()); }

	const std::wstring& getBlendTarget(uint32_t blendTargetIndex) { return m_blendTargets[blendTargetIndex]; }

	void setBlendTargetPosition(uint32_t blendTargetIndex, uint32_t positionIndex, const Vector4& position);

	const Vector4& getBlendTargetPosition(uint32_t blendTargetIndex, uint32_t positionIndex) const;

private:
	std::vector< Material > m_materials;
	std::vector< Vertex > m_vertices;
	std::vector< Polygon > m_polygons;
	Grid3< Vector4 > m_positions;
	Grid3< Vector4 > m_colors; 
	Grid3< Vector4 > m_normals;
	Grid2< Vector2 > m_texCoords;
	std::vector< std::wstring > m_joints;
	std::vector< std::wstring > m_blendTargets;
	std::map< uint32_t, AlignedVector< Vector4 > > m_blendTargetPositions;
};

	}
}

#endif	// traktor_model_Model_H
