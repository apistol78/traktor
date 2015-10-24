#include "Spark/Character.h"
#include "Spark/IComponent.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Character", Character, Object)

Character::Character(const Character* parent)
:	m_parent(parent)
,	m_transform(Matrix33::identity())
,	m_visible(true)
{
}

const Character* Character::getParent() const
{
	return m_parent;
}

void Character::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Character::getName() const
{
	return m_name;
}

void Character::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

const Matrix33& Character::getTransform() const
{
	return m_transform;
}

Matrix33 Character::getFullTransform() const
{
	Matrix33 T = m_transform;
	for (const Character* parent = m_parent; parent; parent = parent->m_parent)
		T = parent->getTransform() * T;
	return T;
}

void Character::setPosition(const Vector2& position)
{
	Vector2 scale;
	float rotation;
	decomposeTransform(m_transform, 0, &scale, &rotation);
	m_transform = composeTransform(position, scale, rotation);
}

Vector2 Character::getPosition() const
{
	Vector2 position;
	decomposeTransform(m_transform, &position, 0, 0);
	return position;
}

void Character::setScale(const Vector2& scale)
{
	Vector2 position;
	float rotation;
	decomposeTransform(m_transform, &position, 0, &rotation);
	m_transform = composeTransform(position, scale, rotation);
}

Vector2 Character::getScale() const
{
	Vector2 scale;
	decomposeTransform(m_transform, 0, &scale, 0);
	return scale;
}

void Character::setRotation(float rotation)
{
	Vector2 position;
	Vector2 scale;
	decomposeTransform(m_transform, &position, &scale, 0);
	m_transform = composeTransform(position, scale, rotation);
}

float Character::getRotation() const
{
	float rotation;
	decomposeTransform(m_transform, 0, 0, &rotation);
	return rotation;
}

void Character::setVisible(bool visible)
{
	m_visible = visible;
}

bool Character::getVisible() const
{
	return m_visible;
}

void Character::eventKey(wchar_t unicode)
{
}

void Character::eventKeyDown(int32_t keyCode)
{
}

void Character::eventKeyUp(int32_t keyCode)
{
}

void Character::eventMouseDown(const Vector2& position, int32_t button)
{
}

void Character::eventMouseUp(const Vector2& position, int32_t button)
{
}

void Character::eventMouseMove(const Vector2& position, int32_t button)
{
}

void Character::eventMouseWheel(const Vector2& position, int32_t delta)
{
}

void Character::eventViewResize(int32_t width, int32_t height)
{
}

	}
}
