#include "Core/Serialization/AttributeAngles.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "World/Entity/CameraEntityData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.CameraEntityData", 0, CameraEntityData, EntityData)

CameraEntityData::CameraEntityData()
:	m_type(CtOrthographic)
,	m_fov(deg2rad(70.0f))
,	m_width(1.0f)
,	m_height(1.0f)
{
}

void CameraEntityData::setCameraType(CameraType type)
{
	m_type = type;
}

CameraType CameraEntityData::getCameraType() const
{
	return m_type;
}

void CameraEntityData::setFieldOfView(float fov)
{
	m_fov = fov;
}

float CameraEntityData::getFieldOfView() const
{
	return m_fov;
}

void CameraEntityData::setWidth(float width)
{
	m_width = width;
}

float CameraEntityData::getWidth() const
{
	return m_width;
}

void CameraEntityData::setHeight(float height)
{
	m_height = height;
}

float CameraEntityData::getHeight() const
{
	return m_height;
}

void CameraEntityData::serialize(ISerializer& s)
{
	const MemberEnum< CameraType >::Key kCameraType[] =
	{
		{ L"CtOrthographic", CtOrthographic },
		{ L"CtPerspective", CtPerspective },
		{ 0, 0 }
	};

	EntityData::serialize(s);

	s >> MemberEnum< CameraType >(L"type", m_type, kCameraType);
	s >> Member< float >(L"fov", m_fov, AttributeAngles());
	s >> Member< float >(L"width", m_width);
	s >> Member< float >(L"height", m_height);
}

	}
}
