/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_SHMatrix_H
#define traktor_render_SHMatrix_H

#include <vector>
#include "Core/Serialization/ISerializable.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Spherical harmonics matrix class.
 * \ingroup Render
 */
class T_DLLCLASS SHMatrix : public ISerializable
{
	T_RTTI_CLASS;

public:
	SHMatrix();

	SHMatrix(int32_t rows, int32_t columns);

	inline int32_t getRows() const { return m_rows; }

	inline int32_t getColumns() const { return m_columns; }

	inline int32_t getSHIndex(int32_t i, int32_t r, int32_t c) const { return (r + i * i + i) + (c + i * i + i) * m_rows; }

	inline float r(int32_t i, int32_t r, int32_t c) const { return m_elements[getSHIndex(i, r, c)]; }

	inline float& w(int32_t i, int32_t r, int32_t c) { return m_elements[getSHIndex(i, r, c)]; }

	inline float r(int32_t row, int32_t column) const { return m_elements[row + column * m_rows]; }

	inline float& w(int32_t row, int32_t column) { return m_elements[row + column * m_rows]; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	int32_t m_rows;
	int32_t m_columns;
	std::vector< float > m_elements;
};

	}
}

#endif	// traktor_render_SHMatrix_H
