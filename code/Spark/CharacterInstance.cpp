#include "Spark/CharacterInstance.h"
#include "Spark/IComponentInstance.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

void decomposeTransform(const Matrix33& transform, Vector2* outTranslate, Vector2* outScale, float* outRotation)
{
	if (outTranslate)
	{
		outTranslate->x = transform.e13;
		outTranslate->y = transform.e23;
	}

	if (outScale)
	{
		outScale->x = Vector2(transform.e11, transform.e12).length();
		outScale->y = Vector2(transform.e21, transform.e22).length();
	}

	if (outRotation)
		*outRotation = -std::atan2(transform.e12, transform.e11);
}

Matrix33 composeTransform(const Vector2& translate_, const Vector2& scale_, float rotate_)
{
	return
		translate(translate_.x, translate_.y) *
		scale(scale_.x, scale_.y) *
		rotate(rotate_);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterInstance", CharacterInstance, Object)

CharacterInstance::CharacterInstance(const CharacterInstance* parent)
:	m_parent(parent)
,	m_transform(Matrix33::identity())
,	m_visible(true)
{
}

void CharacterInstance::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& CharacterInstance::getName() const
{
	return m_name;
}

void CharacterInstance::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

const Matrix33& CharacterInstance::getTransform() const
{
	return m_transform;
}

Matrix33 CharacterInstance::getFullTransform() const
{
	Matrix33 T = m_transform;
	for (const CharacterInstance* parent = m_parent; parent; parent = parent->m_parent)
		T = parent->getTransform() * T;
	return T;
}

void CharacterInstance::setPosition(const Vector2& position)
{
	Vector2 scale;
	float rotation;
	decomposeTransform(m_transform, 0, &scale, &rotation);
	m_transform = composeTransform(position, scale, rotation);
}

Vector2 CharacterInstance::getPosition() const
{
	Vector2 position;
	decomposeTransform(m_transform, &position, 0, 0);
	return position;
}

void CharacterInstance::setScale(const Vector2& scale)
{
	Vector2 position;
	float rotation;
	decomposeTransform(m_transform, &position, 0, &rotation);
	m_transform = composeTransform(position, scale, rotation);
}

Vector2 CharacterInstance::getScale() const
{
	Vector2 scale;
	decomposeTransform(m_transform, 0, &scale, 0);
	return scale;
}

void CharacterInstance::setRotation(float rotation)
{
	Vector2 position;
	Vector2 scale;
	decomposeTransform(m_transform, &position, &scale, 0);
	m_transform = composeTransform(position, scale, rotation);
}

float CharacterInstance::getRotation() const
{
	float rotation;
	decomposeTransform(m_transform, 0, 0, &rotation);
	return rotation;
}

void CharacterInstance::setVisible(bool visible)
{
	m_visible = visible;
}

bool CharacterInstance::getVisible() const
{
	return m_visible;
}

void CharacterInstance::eventKey(wchar_t unicode)
{
}

void CharacterInstance::eventKeyDown(int32_t keyCode)
{
}

void CharacterInstance::eventKeyUp(int32_t keyCode)
{
}

void CharacterInstance::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
}

void CharacterInstance::eventMouseUp(int32_t x, int32_t y, int32_t button)
{
}

void CharacterInstance::eventMouseMove(int32_t x, int32_t y, int32_t button)
{
}

void CharacterInstance::eventMouseWheel(int32_t x, int32_t y, int32_t delta)
{
}

void CharacterInstance::eventViewResize(int32_t width, int32_t height)
{
}

	}
}
