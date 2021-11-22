#pragma once

#include "Core/RefArray.h"
#include "Core/Math/Aabb3.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Settings/PropertyGroup.h"
#include "Model/Animation.h"
#include "Model/Types.h"
#include "Model/Grid2.h"
#include "Model/Grid3.h"
#include "Model/HashVector.h"
#include "Model/Joint.h"
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

/*! Intermediate 3D model.
 * \ingroup Model
 */
class T_DLLCLASS Model : public PropertyGroup
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

	/*! \name Materials */
	//!@{

	uint32_t addMaterial(const Material& material);

	uint32_t addUniqueMaterial(const Material& material);

	const Material& getMaterial(uint32_t index) const { return m_materials[index]; }

	uint32_t getMaterialCount() const { return uint32_t(m_materials.size()); }

	void setMaterials(const AlignedVector< Material >& materials) { m_materials = materials; }

	const AlignedVector< Material >& getMaterials() const { return m_materials; }

	AlignedVector< Material >& getMaterials() { return m_materials; }

	//!@}

	/*! \name Vertices */
	//!@{

	void reserveVertices(uint32_t vertexCapacity) { m_vertices.reserve(vertexCapacity); }

	uint32_t addVertex(const Vertex& vertex);

	uint32_t addUniqueVertex(const Vertex& vertex);

	void setVertex(uint32_t index, const Vertex& vertex) { m_vertices.set(index, vertex); }

	const Vertex& getVertex(uint32_t index) const { return m_vertices[index]; }

	uint32_t getVertexCount() const { return uint32_t(m_vertices.size()); }

	void setVertices(const AlignedVector< Vertex >& vertices) { m_vertices.replace(vertices); }

	const AlignedVector< Vertex >& getVertices() const { return m_vertices.values(); }

	//!@}

	/*! \name Polygons */
	//!@{

	void reservePolygons(uint32_t polygonCapacity) { m_polygons.reserve(polygonCapacity); }

	uint32_t addPolygon(const Polygon& polygon);

	uint32_t addUniquePolygon(const Polygon& polygon);

	void setPolygon(uint32_t index, const Polygon& polygon) { m_polygons[index] = polygon; }

	const Polygon& getPolygon(uint32_t index) const { return m_polygons[index]; }

	uint32_t getPolygonCount() const { return uint32_t(m_polygons.size()); }

	void setPolygons(const AlignedVector< Polygon >& polygons) { m_polygons = polygons; }

	const AlignedVector< Polygon >& getPolygons() const { return m_polygons; }

	AlignedVector< Polygon >& getPolygons() { return m_polygons; }

	//!@}

	/*! \name Positions */
	//!@{

	void reservePositions(uint32_t positionCapacity);

	uint32_t addPosition(const Vector4& position);

	uint32_t addUniquePosition(const Vector4& position, float distance = 0.01f);

	uint32_t getPositionCount() const { return m_positions.size(); }

	void setPosition(uint32_t index, const Vector4& position) { m_positions.set(index, position); }

	const Vector4& getPosition(uint32_t index) const { return m_positions.get(index, Vector4::zero()); }

	const Vector4& getVertexPosition(uint32_t vertexIndex) const { return getPosition(getVertex(vertexIndex).getPosition()); }

	void setPositions(const AlignedVector< Vector4 >& positions) { m_positions.replace(positions); }

	const AlignedVector< Vector4 >& getPositions() const { return m_positions.values(); }

	//!@}

	/*! \name Colors */
	//!@{

	uint32_t addColor(const Vector4& color);

	uint32_t addUniqueColor(const Vector4& color);

	uint32_t getColorCount() const { return m_colors.size(); }

	const Vector4& getColor(uint32_t index) const { return m_colors.get(index, Vector4::zero()); }

	void setColors(const AlignedVector< Vector4 >& colors) { m_colors.replace(colors); }

	const AlignedVector< Vector4 >& getColors() const { return m_colors.values(); }

	void reserveColors(uint32_t colorCapacity);

	//!@}

	/*! \name Normals */
	//!@{

	uint32_t addNormal(const Vector4& normal);

	uint32_t addUniqueNormal(const Vector4& normal);

	uint32_t getNormalCount() const { return m_normals.size(); }

	const Vector4& getNormal(uint32_t index) const { return m_normals.get(index, Vector4::zero()); }

	void setNormals(const AlignedVector< Vector4 >& normals) { m_normals.replace(normals); }

	const AlignedVector< Vector4 >& getNormals() const { return m_normals.values(); }

	void reserveNormals(uint32_t normalCapacity);

	//!@}

	/*! \name Texture coordinates. */
	//!@{

	uint32_t addTexCoord(const Vector2& texCoord);

	uint32_t addUniqueTexCoord(const Vector2& texCoord);

	const Vector2& getTexCoord(uint32_t index) const { return m_texCoords.get(index, Vector2::zero()); }

	void setTexCoords(const AlignedVector< Vector2 >& texCoords) { m_texCoords.replace(texCoords); }

	const AlignedVector< Vector2 >& getTexCoords() const { return m_texCoords.values(); }

	uint32_t addUniqueTexCoordChannel(const std::wstring& channelId);

	uint32_t getTexCoordChannel(const std::wstring& channelId) const;

	void setTexCoordChannels(const AlignedVector< std::wstring >& texCoordChannels) { m_texCoordChannels = texCoordChannels; }

	const AlignedVector< std::wstring >& getTexCoordChannels() const { return m_texCoordChannels; }

	//!@}

	/*! \name Skeleton joints. */
	//!@{

	uint32_t addJoint(const Joint& joint);

	uint32_t addUniqueJoint(const Joint& joint);

	uint32_t getJointCount() const { return (uint32_t)m_joints.size(); }

	const Joint& getJoint(uint32_t jointIndex) const { return m_joints[jointIndex]; }

	void setJoints(const AlignedVector< Joint >& joints);

	const AlignedVector< Joint >& getJoints() const { return m_joints; }

	uint32_t findJointIndex(const std::wstring& jointName) const;

	void findChildJoints(uint32_t jointId, AlignedVector< uint32_t >& outChildJoints) const;

	Transform getJointGlobalTransform(uint32_t jointId) const;

	void setJointRotation(uint32_t jointId, const Quaternion& rotation);

	//!@}

	/*! \name Animations. */
	//!@{

	uint32_t addAnimation(Animation* animation);

	uint32_t getAnimationCount() const { return (uint32_t)m_animations.size(); }

	const Animation* getAnimation(uint32_t animationIndex) const { return m_animations[animationIndex]; }

	const Animation* findAnimation(const std::wstring& animationName) const;

	const RefArray< Animation >& getAnimations() const { return m_animations; }

	//!@}

	/*! \name Blend targets, aka morphs. */
	//!@{

	uint32_t addBlendTarget(const std::wstring& blendTargetName);

	uint32_t getBlendTargetCount() const { return (uint32_t)m_blendTargets.size(); }

	const std::wstring& getBlendTarget(uint32_t blendTargetIndex) { return m_blendTargets[blendTargetIndex]; }

	void setBlendTargetPosition(uint32_t blendTargetIndex, uint32_t positionIndex, const Vector4& position);

	const Vector4& getBlendTargetPosition(uint32_t blendTargetIndex, uint32_t positionIndex) const;

	//!@}

	virtual void serialize(ISerializer& s) override final;

private:
	AlignedVector< Material > m_materials;
	HashVector< Vertex, VertexHashFunction > m_vertices;
	AlignedVector< Polygon > m_polygons;
	Grid3< Vector4 > m_positions;
	Grid3< Vector4 > m_colors;
	Grid3< Vector4 > m_normals;
	Grid2< Vector2 > m_texCoords;
	AlignedVector< std::wstring > m_texCoordChannels;
	AlignedVector< Joint > m_joints;
	RefArray< Animation > m_animations;
	AlignedVector< std::wstring > m_blendTargets;
	SmallMap< uint32_t, AlignedVector< Vector4 > > m_blendTargetPositions;

	void validate() const;
};

	}
}
