#pragma once

#include "Spark/Action/ActionTypes.h"
#include "Spark/Action/Common/BitmapFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class T_DLLCLASS BlurFilter : public BitmapFilter
{
	T_RTTI_CLASS;

public:
	BlurFilter(float blurX, float blurY, int32_t quality);

	virtual Ref< BitmapFilter > clone() override final;

	float getBlurX();

	void setBlurX(float blurX);

	float getBlurY();

	void setBlurY(float blurY);

	int32_t getQuality();

	void setQuality(int32_t quality);

private:
	float m_blurX;
	float m_blurY;
	int32_t m_quality;
};

	}
}

