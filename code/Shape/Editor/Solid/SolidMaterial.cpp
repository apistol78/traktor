#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/TextureAsset.h"
#include "Shape/Editor/Solid/SolidMaterial.h"

namespace traktor
{
	namespace shape
	{
	
T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.SolidMaterial", 0, SolidMaterial, ISerializable)
	
void SolidMaterial::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"albedo", m_albedo, AttributeType(type_of< render::TextureAsset >()));
	s >> Member< Guid >(L"normal", m_normal, AttributeType(type_of< render::TextureAsset >()));
	s >> Member< Guid >(L"roughness", m_roughness, AttributeType(type_of< render::TextureAsset >()));
	s >> Member< Guid >(L"metalness", m_metalness, AttributeType(type_of< render::TextureAsset >()));
}

	}
}