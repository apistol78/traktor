#include "UnitTest/CaseSpatialHashArray.h"
#include "Model/SpatialHashArray.h"

namespace traktor
{

void CaseSpatialHashArray::run()
{
	{
		model::SpatialHashArray< Vector2 > vector2Array;
		uint32_t id1 = vector2Array.add(Vector2( 1.0f, 0.0f));
		uint32_t id2 = vector2Array.add(Vector2(-1.0f, 0.0f));
		CASE_ASSERT_NOT_EQUAL(id1, id2);
		CASE_ASSERT_EQUAL(id1, 0);
		CASE_ASSERT_EQUAL(id2, 1);
	}

	{
		model::SpatialHashArray< Vector2 > vector2Array;
		uint32_t id1 = vector2Array.add(Vector2(0.0f,  1.0f));
		uint32_t id2 = vector2Array.add(Vector2(0.0f, -1.0f));
		CASE_ASSERT_NOT_EQUAL(id1, id2);
		CASE_ASSERT_EQUAL(id1, 0);
		CASE_ASSERT_EQUAL(id2, 1);
	}

	{
		model::SpatialHashArray< Vector4 > vector4Array;
		uint32_t id1 = vector4Array.add(Vector4( 1.0f, 0.0f, 0.0f, 0.0f));
		uint32_t id2 = vector4Array.add(Vector4(-1.0f, 0.0f, 0.0f, 0.0f));
		CASE_ASSERT_NOT_EQUAL(id1, id2);
		CASE_ASSERT_EQUAL(id1, 0);
		CASE_ASSERT_EQUAL(id2, 1);
	}

	{
		model::SpatialHashArray< Vector4 > vector4Array;
		uint32_t id1 = vector4Array.add(Vector4(0.0f,  1.0f, 0.0f, 0.0f));
		uint32_t id2 = vector4Array.add(Vector4(0.0f, -1.0f, 0.0f, 0.0f));
		CASE_ASSERT_NOT_EQUAL(id1, id2);
		CASE_ASSERT_EQUAL(id1, 0);
		CASE_ASSERT_EQUAL(id2, 1);
	}

	{
		model::SpatialHashArray< Vector4 > vector4Array;
		uint32_t id1 = vector4Array.add(Vector4(0.0f, 0.0f,  1.0f, 0.0f));
		uint32_t id2 = vector4Array.add(Vector4(0.0f, 0.0f, -1.0f, 0.0f));
		CASE_ASSERT_NOT_EQUAL(id1, id2);
		CASE_ASSERT_EQUAL(id1, 0);
		CASE_ASSERT_EQUAL(id2, 1);
	}
}

}
