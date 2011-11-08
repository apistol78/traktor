#ifndef traktor_flash_AsStage_H
#define traktor_flash_AsStage_H

#include "Flash/SwfTypes.h"
#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Stage class.
 * \ingroup Flash
 */
class AsStage : public ActionObject
{
	T_RTTI_CLASS;

public:
	AsStage(ActionContext* context);

	void eventResize(int32_t width, int32_t height);

	SwfAlignType getAlignH() const { return m_alignH; }

	SwfAlignType getAlignV() const { return m_alignV; }

	SwfScaleModeType getScaleMode() const { return m_scaleMode; }

private:
	int32_t m_width;
	int32_t m_height;
	SwfAlignType m_alignH;
	SwfAlignType m_alignV;
	SwfScaleModeType m_scaleMode;

	void Stage_get_align(CallArgs& ca);

	void Stage_set_align(CallArgs& ca);

	void Stage_get_height(CallArgs& ca);

	void Stage_get_scaleMode(CallArgs& ca);

	void Stage_set_scaleMode(CallArgs& ca);

	void Stage_get_showMenu(CallArgs& ca);

	void Stage_set_showMenu(CallArgs& ca);

	void Stage_get_width(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsStage_H
