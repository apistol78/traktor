#if defined(_WIN32)
#	include <windows.h>
#endif
#include "Core/Log/Log.h"
#include "UnitTest/CaseAtomic.h"
#include "UnitTest/CaseBitReader.h"
#include "UnitTest/CaseBodyState.h"
#include "UnitTest/CaseClone.h"
#include "UnitTest/CaseCycleDebugger.h"
#include "UnitTest/CaseMeta.h"
#include "UnitTest/CaseProcess.h"
#include "UnitTest/CaseThread.h"
#include "UnitTest/CaseJob.h"
#include "UnitTest/CaseMath.h"
#include "UnitTest/CaseMatrix33.h"
#include "UnitTest/CaseMatrix44.h"
#include "UnitTest/CaseSmallMap.h"
#include "UnitTest/CaseQuaternion.h"
#include "UnitTest/CaseTransform.h"
#include "UnitTest/CaseValueTemplate.h"
#include "UnitTest/CaseWinding.h"
#include "UnitTest/CasePath.h"
#include "UnitTest/CaseXmlPullParser.h"
#include "UnitTest/CaseXmlDocument.h"
#include "UnitTest/CaseZip.h"
#include "UnitTest/CaseRenderSw.h"
#include "UnitTest/CaseSound.h"
#include "UnitTest/CaseStreamCopy.h"

using namespace traktor;

#if !defined(_WIN32) || defined(_CONSOLE)
int main(int /*argc*/, const char** /*argv*/)
#elif defined(WINCE)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{
	Case::Context context;

	//log::info << L"Case clone" << Endl;
	//traktor::CaseClone().execute(context);

	//log::info << L"Case cycle debugger" << Endl;
	//traktor::CaseCycleDebugger().execute(context);

	//log::info << L"Case atomic" << Endl;
	//traktor::CaseAtomic().execute(context);

	//log::info << L"Case bitreader" << Endl;
	//traktor::CaseBitReader().execute(context);

	//log::info << L"Case bodystate" << Endl;
	//traktor::CaseBodyState().execute(context);

	//log::info << L"Case meta" << Endl;
	//traktor::CaseMeta().execute(context);

	////log::info << L"Case process" << Endl;
	////traktor::CaseProcess().execute(context);

	//log::info << L"Case thread" << Endl;
	//traktor::CaseThread().execute(context);

	//log::info << L"Case job" << Endl;
	//traktor::CaseJob().execute(context);

	//log::info << L"Case math" << Endl;
	//traktor::CaseMath().execute(context);

	//log::info << L"Case matrix 3x3" << Endl;
	//traktor::CaseMatrix33().execute(context);

	//log::info << L"Case matrix 4x4" << Endl;
	//traktor::CaseMatrix44().execute(context);

	//log::info << L"Case quaternion" << Endl;
	//traktor::CaseQuaternion().execute(context);

	////log::info << L"Case small map" << Endl;
	////traktor::CaseSmallMap().execute(context);

	//log::info << L"Case transform" << Endl;
	//traktor::CaseTransform().execute(context);

	log::info << L"Case value template" << Endl;
	traktor::CaseValueTemplate().execute(context);

	//log::info << L"Case winding" << Endl;
	//traktor::CaseWinding().execute(context);

	//log::info << L"Case path" << Endl;
	//traktor::CasePath().execute(context);

	//log::info << L"Case xml pull parser" << Endl;
	//traktor::CaseXmlPullParser().execute(context);

	////log::info << L"Case xml document" << Endl;
	////traktor::CaseXmlDocument().execute(context);

	//log::info << L"Case zip" << Endl;
	//traktor::CaseZip().execute(context);

	//log::info << L"Case software renderer" << Endl;
	//traktor::CaseRenderSw().execute(context);

	////log::info << L"Case sound" << Endl;
	////traktor::CaseSound().execute(context);

	////log::info << L"Case stream copy" << Endl;
	////traktor::CaseStreamCopy().execute(context);

	log::info << L"Finished" << Endl;
	log::info << IncreaseIndent;
	log::info << context.succeeded << L" succeeded" << Endl;
	log::info << context.failed << L" failed" << Endl;
	log::info << DecreaseIndent;

	return context.failed == 0 ? 0 : 1;
}
