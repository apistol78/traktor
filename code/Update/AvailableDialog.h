#ifndef traktor_update_AvailableDialog_H
#define traktor_update_AvailableDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace update
	{

class Bundle;

/*! \brief Update available dialog.
 * \ingroup Update
 */
class AvailableDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(const Bundle* bundle);
};

	}
}

#endif	// traktor_update_AvailableDialog_H
