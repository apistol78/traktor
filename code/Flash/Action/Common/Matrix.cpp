#include "Core/Io/StringOutputStream.h"
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Common/Matrix.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Matrix", Matrix, ActionObjectRelay)

Matrix::Matrix()
:	ActionObjectRelay("Matrix")
,	m_v(Matrix33::identity())
{
}

Matrix::Matrix(const float v[6])
:	ActionObjectRelay("Matrix")
{
	m_v.e11 = v[0]; m_v.e12 = v[1]; m_v.e13 = v[4];
	m_v.e21 = v[2]; m_v.e22 = v[3]; m_v.e23 = v[5];
	m_v.e31 = 0.0f; m_v.e32 = 0.0f; m_v.e33 = 1.0f;
}

Ref< Matrix > Matrix::clone()
{
	Ref< Matrix > c = new Matrix();
	c->m_v = m_v;
	return c;
}

void Matrix::concat(const Matrix* rh)
{
	m_v *= rh->m_v;
}

void Matrix::createBox(float scaleX, float scaleY, float rotation, float tx, float ty)
{
	T_FATAL_ERROR;
}

void Matrix::createGradientBox(float width, float height, float rotation, float tx, float ty)
{
	float x = tx;
	float y = ty;
	float w = width;
	float h = height;
	float r = rotation;
	m_v =
		traktor::translate(10.0f * w + 20.0f * x, 10.0f * h + 20.0f * y) *
		traktor::rotate(r) *
		traktor::scale(w / 2000.0f, h / 2000.0f);
}

Ref< Point > Matrix::deltaTransformPoint(const Point* pt)
{
	T_FATAL_ERROR;
	return 0;
}

void Matrix::identity()
{
	m_v = Matrix33::identity();
}

void Matrix::invert()
{
	m_v = m_v.inverse();
}

void Matrix::rotate(float angle)
{
	m_v *= traktor::rotate(angle);
}

void Matrix::scale(float scaleX, float scaleY)
{
	m_v *= traktor::scale(scaleX, scaleY);
}

std::wstring Matrix::toString()
{
	StringOutputStream ss;
	ss << L"(a=" << m_v.e11 << L", b=" << m_v.e12 << L", c=" << m_v.e21 << L", d=" << m_v.e22 << L", tx=" << m_v.e13 << L", ty=" << m_v.e23 << L")";
	return ss.str();
}

Ref< Point > Matrix::transformPoint(const Point* pt)
{
	T_FATAL_ERROR;
	return 0;
}

void Matrix::translate(float x, float y)
{
	m_v *= traktor::translate(x, y);
}

	}
}
