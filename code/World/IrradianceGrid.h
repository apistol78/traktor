#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace render
    {

class StructBuffer;

    }

    namespace world
    {

#pragma pack(1)

/*! Irradiance grid cell data.
 * \ingroup World
 */
struct IrradianceGridData
{
	float shR0_3[4];
	float shR4_7[4];
	float shG0_3[4];
	float shG4_7[4];
	float shB0_3[4];
	float shB4_7[4];
	float shRGB_8[4];
};

#pragma pack()

/*! Irradiance grid.
 * \ingroup World
 */
class T_DLLCLASS IrradianceGrid : public Object
{
    T_RTTI_CLASS;

public:
    IrradianceGrid(const Vector4& size, render::StructBuffer* buffer);

    const Vector4& getSize() const { return m_size; }

    render::StructBuffer* getBuffer() const { return m_buffer; }

private:
    Vector4 m_size;
    Ref< render::StructBuffer > m_buffer;
};

    }
}