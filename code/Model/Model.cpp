#include <algorithm>
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Model/ContainerHelpers.h"
#include "Model/Model.h"
#include "Model/Pose.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

/*! Return true if a replacing vertex match or "exceed" an existing vertex. */
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.Model", 0, Model, ISerializable)

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

	for (AlignedVector< Vertex >::iterator i = m_vertices.begin(); i != m_vertices.end(); ++i)
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

	for (AlignedVector< Polygon >::iterator i = m_polygons.begin(); i != m_polygons.end(); ++i)
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

uint32_t Model::addPolygon(const Polygon& polygon)
{
	return addId(m_polygons, polygon);
}

uint32_t Model::addUniquePolygon(const Polygon& polygon)
{
	return addUniqueId< AlignedVector< Polygon >, Polygon, DefaultPredicate< Polygon > >(m_polygons, polygon);
}

void Model::reservePositions(uint32_t positionCapacity)
{
	m_positions.reserve(positionCapacity);
}

uint32_t Model::addPosition(const Vector4& position)
{
	return m_positions.add(position);
}

uint32_t Model::addUniquePosition(const Vector4& position, float distance)
{
	uint32_t id = m_positions.get(position, distance);
	return id != m_positions.InvalidIndex ? id : m_positions.add(position);
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

void Model::reserveColors(uint32_t colorCapacity)
{
	m_colors.reserve(colorCapacity);
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

void Model::reserveNormals(uint32_t normalCapacity)
{
	m_normals.reserve(normalCapacity);
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

uint32_t Model::addUniqueTexCoordChannel(const std::wstring& channelId)
{
	uint32_t id = getTexCoordChannel(channelId);
	if (id == c_InvalidIndex)
	{
		id = (uint32_t)m_texCoordChannels.size();
		m_texCoordChannels.push_back(channelId);
	}
	return id;
}

uint32_t Model::getTexCoordChannel(const std::wstring& channelId) const
{
	const auto it = std::find(m_texCoordChannels.begin(), m_texCoordChannels.end(), channelId);
	return it != m_texCoordChannels.end() ? (uint32_t)std::distance(m_texCoordChannels.begin(), it) : c_InvalidIndex;
}

uint32_t Model::addJoint(const Joint& joint)
{
	return addId(m_joints, joint);
}

uint32_t Model::addUniqueJoint(const Joint& joint)
{
	return addUniqueId< AlignedVector< Joint >, Joint, DefaultPredicate< Joint > >(m_joints, joint);
}

void Model::setJoints(const AlignedVector< Joint >& joints)
{
	m_joints = joints;
}

uint32_t Model::findJointIndex(const std::wstring& jointName) const
{
	const auto i = std::find_if(m_joints.begin(), m_joints.end(), [&](const Joint& j) {
		return j.getName() == jointName;
	});
	return i != m_joints.end() ? uint32_t(std::distance(m_joints.begin(), i)) : c_InvalidIndex;
}

void Model::findChildJoints(uint32_t jointId, AlignedVector< uint32_t >& outChildJoints) const
{
	for (uint32_t i = 0; i < m_joints.size(); ++i)
	{
		if (m_joints[i].getParent() == jointId)
			outChildJoints.push_back(i);
	}
}

Transform Model::getJointGlobalTransform(uint32_t jointId) const
{
	Transform Tglobal = Transform::identity();
	while (jointId != c_InvalidIndex)
	{
		Tglobal = m_joints[jointId].getTransform() * Tglobal;	// ABC order (A root)
		jointId = m_joints[jointId].getParent();
	}
	return Tglobal;
}

void Model::setJointRotation(uint32_t jointId, const Quaternion& rotation)
{
	Joint& joint = m_joints[jointId];

	Transform Tcurr = joint.getTransform();
	Transform Tnext(Tcurr.translation(), rotation);
	joint.setTransform(Tnext);

	for (uint32_t i = 0; i < (uint32_t)m_joints.size(); ++i)
	{
		Joint& child = m_joints[i];
		if (child.getParent() == jointId)
		{
			Transform Tchild = Tnext.inverse() * Tcurr * child.getTransform();
			child.setTransform(Tchild);
		}
	}

	for (auto animation : m_animations)
	{
		for (uint32_t i = 0; i < animation->getKeyFrameCount(); ++i)
		{
			Ref< Pose > pose = new Pose(*animation->getKeyFramePose(i));

			Transform TposeCurr = pose->getJointTransform(jointId);

			Quaternion QrotationDelta = TposeCurr.rotation() * Tcurr.rotation().inverse();
			Transform TposeNew(
				TposeCurr.translation(),
				QrotationDelta * rotation
			);

			pose->setJointTransform(jointId, TposeNew);

			Transform TposeDelta = TposeNew.inverse() * TposeCurr;
			for (uint32_t j = 0; j < (uint32_t)m_joints.size(); ++j)
			{
				const Joint& child = m_joints[j];
				if (child.getParent() == jointId)
				{
					Transform tmp0 = pose->getJointTransform(j);
					Transform tmp1 = TposeNew.inverse() * TposeCurr * tmp0;
					pose->setJointTransform(j, tmp1);
				}
			}

			animation->setKeyFramePose(i, pose);
		}
	}
}

uint32_t Model::addAnimation(Animation* animation)
{
	uint32_t id = (uint32_t)m_animations.size();
	m_animations.push_back(animation);
	return id;
}

const Animation* Model::findAnimation(const std::wstring& animationName) const
{
	const auto i = std::find_if(m_animations.begin(), m_animations.end(), [&](const Animation* anim) {
		return anim->getName() == animationName;
	});
	return i != m_animations.end() ? *i : nullptr;
}

uint32_t Model::addBlendTarget(const std::wstring& blendTargetName)
{
	const auto i = std::find(m_blendTargets.begin(), m_blendTargets.end(), blendTargetName);
	if (i != m_blendTargets.end())
		return uint32_t(std::distance(m_blendTargets.begin(), i));

	m_blendTargets.push_back(blendTargetName);
	uint32_t id = uint32_t(m_blendTargets.size() - 1);

	m_blendTargetPositions[id] = m_positions.values();
	return id;
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
	auto it = m_blendTargetPositions.find(blendTargetIndex);
	if (it != m_blendTargetPositions.end())
		return it->second[positionIndex];
	else
		return Vector4::origo();
}

void Model::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Material, MemberComposite< Material > >(L"materials", m_materials);
	s >> MemberAlignedVector< Vertex, MemberComposite< Vertex > >(L"vertices", m_vertices);
	s >> MemberAlignedVector< Polygon, MemberComposite< Polygon > >(L"polygons", m_polygons);

	AlignedVector< Vector4 > positions = m_positions.values();
	s >> MemberAlignedVector< Vector4 >(L"positions", positions);
	m_positions.replace(positions);

	AlignedVector< Vector4 > colors = m_colors.values();
	s >> MemberAlignedVector< Vector4 >(L"colors", colors);
	m_colors.replace(colors);

	AlignedVector< Vector4 > normals = m_normals.values();
	s >> MemberAlignedVector< Vector4 >(L"normals", normals);
	m_normals.replace(normals);

	AlignedVector< Vector2 > texCoords = m_texCoords.values();
	s >> MemberAlignedVector< Vector2 >(L"texCoords", texCoords);
	m_texCoords.replace(texCoords);

	s >> MemberAlignedVector< std::wstring >(L"texCoordChannels", m_texCoordChannels);

	s >> MemberAlignedVector< Joint, MemberComposite< Joint > >(L"joints", m_joints);

	s >> MemberRefArray< Animation >(L"animations", m_animations);

	s >> MemberAlignedVector< std::wstring >(L"blendTargets", m_blendTargets);

	s >> MemberSmallMap<
		uint32_t,
		AlignedVector< Vector4 >,
		Member< uint32_t >,
		MemberAlignedVector< Vector4 >
	>(L"blendTargetPositions", m_blendTargetPositions);
}

	}
}
