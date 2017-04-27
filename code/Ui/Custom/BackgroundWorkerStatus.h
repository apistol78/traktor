/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_BackgroundWorkerStatus_H
#define traktor_ui_custom_BackgroundWorkerStatus_H

#include <list>
#include "Core/Thread/Semaphore.h"
#include "Ui/Custom/BackgroundWorkerDialog.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Background worker status report.
 * \ingroup UIC
 */
class T_DLLCLASS BackgroundWorkerStatus : public RefCountImpl< BackgroundWorkerDialog::IWorkerStatus >
{
public:
	BackgroundWorkerStatus();

	BackgroundWorkerStatus(int32_t steps);

	void setSteps(int32_t steps);

	void notify(int32_t step, const std::wstring& status);

	virtual bool read(int32_t& outStep, std::wstring& outStatus) T_OVERRIDE T_FINAL;

private:
	Semaphore m_lock;
	int32_t m_steps;
	int32_t m_step;
	std::wstring m_status;
};

		}
	}
}

#endif	// traktor_ui_custom_BackgroundWorkerStatus_H
