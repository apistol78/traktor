#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Math/Aabb3.h"
#include "Core/Math/Half.h"

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
#if !defined(__ANDROID__)
	half_t shR0_3[4];   // 0-8
	half_t shR4_7[4];   // 8-16
	half_t shG0_3[4];   // 16-24
	half_t shG4_7[4];   // 24-32
	half_t shB0_3[4];   // 32-40
	half_t shB4_7[4];   // 40-48
	half_t shRGB_8[4];  // 48-56
    uint8_t pad[8];     // 56-64
#else
	float shR0_3[4];
	float shR4_7[4];
	float shG0_3[4];
	float shG4_7[4];
	float shB0_3[4];
	float shB4_7[4];
	float shRGB_8[4];
#endif
};

#pragma pack()

/*! Irradiance grid.
 * \ingroup World
 */
class T_DLLCLASS IrradianceGrid : public Object
{
    T_RTTI_CLASS;

public:
    typedef uint32_t gridSize_t [3];

    IrradianceGrid(
        gridSize_t size,
        const Aabb3& boundingBox,
        render::StructBuffer* buffer
    );

    const gridSize_t& getSize() const { return m_size; }

    const Aabb3& getBoundingBox() const { return m_boundingBox; }

    render::StructBuffer* getBuffer() const { return m_buffer; }

private:
    gridSize_t m_size;
    Aabb3 m_boundingBox;
    Ref< render::StructBuffer > m_buffer;
};

    }
}