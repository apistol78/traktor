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

Matrix::Matrix(const avm_number_t v[6])
:	ActionObjectRelay("Matrix")
{
	m_v.e11 = v[0]; m_v.e12 = v[1]; m_v.e13 = v[4];
	m_v.e11 = v[2]; m_v.e12 = v[3]; m_v.e13 = v[5];
	m_v.e11 = 0.0f; m_v.e12 = 0.0f; m_v.e13 = 1.0f;
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

void Matrix::createBox(avm_number_t scaleX, avm_number_t scaleY, avm_number_t rotation, avm_number_t tx, avm_number_t ty)
{
	T_FATAL_ERROR;
}

void Matrix::createGradientBox(avm_number_t width, avm_number_t height, avm_number_t rotation, avm_number_t tx, avm_number_t ty)
{
	T_FATAL_ERROR;
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

void Matrix::rotate(avm_number_t angle)
{
	m_v *= traktor::rotate(angle);
}

void Matrix::scale(avm_number_t scaleX, avm_number_t scaleY)
{
	m_v *= traktor::scale(scaleX, scaleY);
}

std::wstring Matrix::toString()
{
	return L"N/A";
}

Ref< Point > Matrix::transformPoint(const Point* pt)
{
	T_FATAL_ERROR;
	return 0;
}

void Matrix::translate(avm_number_t x, avm_number_t y)
{
	m_v *= traktor::translate(x, y);
}

	}
}
