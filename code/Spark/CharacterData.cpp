/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/CharacterData.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterData", CharacterData, ISerializable)

CharacterData::CharacterData()
:	m_transform(Matrix33::identity())
{
}

void CharacterData::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

const Matrix33& CharacterData::getTransform() const
{
	return m_transform;
}

void CharacterData::serialize(ISerializer& s)
{
	s >> Member< Matrix33 >(L"transform", m_transform);
}

	}
}
