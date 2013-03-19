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

	const Material& getMaterial(uint32_t index) const;

	void setMaterials(const std::vector< Material >& materials);

	const std::vector< Material >& getMaterials() const;

	uint32_t addVertex(const Vertex& vertex);

	uint32_t addUniqueVertex(const Vertex& vertex);

	void setVertex(uint32_t index, const Vertex& vertex);

	const Vertex& getVertex(uint32_t index) const;

	uint32_t getVertexCount() const;

	void setVertices(const std::vector< Vertex >& vertices);

	const std::vector< Vertex >& getVertices() const;

	void reservePolygons(uint32_t polygonCapacity);

	uint32_t addPolygon(const Polygon& polygon);

	uint32_t addUniquePolygon(const Polygon& polygon);

	void setPolygon(uint32_t index, const Polygon& polygon);

	const Polygon& getPolygon(uint32_t index) const;

	uint32_t getPolygonCount() const;

	void setPolygons(const std::vector< Polygon >& polygons);

	const std::vector< Polygon >& getPolygons() const;

	void reservePositions(uint32_t positionCapacity);

	uint32_t addPosition(const Vector4& position);

	uint32_t addUniquePosition(const Vector4& position);

	const Vector4& getPosition(uint32_t index) const;

	const Vector4& getVertexPosition(uint32_t vertexIndex) const;

	void setPositions(const AlignedVector< Vector4 >& positions);

	const AlignedVector< Vector4 >& getPositions() const;

	uint32_t addColor(const Vector4& color);

	uint32_t addUniqueColor(const Vector4& color);

	const Vector4& getColor(uint32_t index) const;

	void setColors(const AlignedVector< Vector4 >& colors);

	const AlignedVector< Vector4 >& getColors() const;

	uint32_t addNormal(const Vector4& normal);

	uint32_t addUniqueNormal(const Vector4& normal);

	const Vector4& getNormal(uint32_t index) const;

	void setNormals(const AlignedVector< Vector4 >& normals);

	const AlignedVector< Vector4 >& getNormals() const;

	uint32_t addTexCoord(const Vector2& texCoord);

	uint32_t addUniqueTexCoord(const Vector2& texCoord);

	const Vector2& getTexCoord(uint32_t index) const;

	void setTexCoords(const AlignedVector< Vector2 >& texCoords);

	const AlignedVector< Vector2 >& getTexCoords() const;

	uint32_t addJoint(const std::wstring& jointName);

	uint32_t getJointCount() const;

	const std::wstring& getJoint(uint32_t jointIndex) const;

	uint32_t findJointIndex(const std::wstring& jointName) const;

	uint32_t addBlendTarget(const std::wstring& blendTargetName);

	uint32_t getBlendTargetCount() const;

	const std::wstring& getBlendTarget(uint32_t blendTargetIndex);

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
