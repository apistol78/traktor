#ifndef traktor_flash_BlurFilter_H
#define traktor_flash_BlurFilter_H

#include "Flash/Action/ActionTypes.h"
#include "Flash/Action/Common/BitmapFilter.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS BlurFilter : public BitmapFilter
{
	T_RTTI_CLASS;

public:
	BlurFilter(avm_number_t blurX, avm_number_t blurY, int32_t quality);

	virtual Ref< BitmapFilter > clone() T_OVERRIDE T_FINAL;

	avm_number_t getBlurX();

	void setBlurX(avm_number_t blurX);

	avm_number_t getBlurY();

	void setBlurY(avm_number_t blurY);

	int32_t getQuality();

	void setQuality(int32_t quality);

private:
	avm_number_t m_blurX;
	avm_number_t m_blurY;
	int32_t m_quality;
};

	}
}

#endif	// traktor_flash_BlurFilter_H
