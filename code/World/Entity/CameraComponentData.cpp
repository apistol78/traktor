#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeAngles.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "World/Entity/CameraComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.CameraComponentData", 0, CameraComponentData, IEntityComponentData)

CameraComponentData::CameraComponentData()
:	m_type(CtOrthographic)
,	m_fov(deg2rad(70.0f))
,	m_width(1.0f)
,	m_height(1.0f)
{
}

void CameraComponentData::setCameraType(CameraType type)
{
	m_type = type;
}

CameraType CameraComponentData::getCameraType() const
{
	return m_type;
}

void CameraComponentData::setFieldOfView(float fov)
{
	m_fov = fov;
}

float CameraComponentData::getFieldOfView() const
{
	return m_fov;
}

void CameraComponentData::setWidth(float width)
{
	m_width = width;
}

float CameraComponentData::getWidth() const
{
	return m_width;
}

void CameraComponentData::setHeight(float height)
{
	m_height = height;
}

float CameraComponentData::getHeight() const
{
	return m_height;
}

void CameraComponentData::serialize(ISerializer& s)
{
	const MemberEnum< CameraType >::Key kCameraType[] =
	{
		{ L"CtOrthographic", CtOrthographic },
		{ L"CtPerspective", CtPerspective },
		{ 0, 0 }
	};

	s >> MemberEnum< CameraType >(L"type", m_type, kCameraType);
	s >> Member< float >(L"fov", m_fov, AttributeAngles());
	s >> Member< float >(L"width", m_width);
	s >> Member< float >(L"height", m_height);
}

	}
}
