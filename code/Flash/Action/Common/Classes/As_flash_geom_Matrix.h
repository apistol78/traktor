/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_As_flash_geom_Matrix_H
#define traktor_flash_As_flash_geom_Matrix_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Matrix class.
 * \ingroup Flash
 */
class As_flash_geom_Matrix : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_geom_Matrix(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	void Matrix_distance(CallArgs& ca);

	void Matrix_interpolate(CallArgs& ca);

	void Matrix_polar(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_flash_geom_Matrix_H
