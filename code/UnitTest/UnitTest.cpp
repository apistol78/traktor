#if defined(_WIN32)
#	include <windows.h>
#endif
#include "UnitTest/CaseAtomic.h"
#include "UnitTest/CaseCycleDebugger.h"
#include "UnitTest/CaseMeta.h"
#include "UnitTest/CaseProcess.h"
#include "UnitTest/CaseThread.h"
#include "UnitTest/CaseJob.h"
#include "UnitTest/CaseHeap.h"
#include "UnitTest/CaseMath.h"
#include "UnitTest/CaseMatrix33.h"
#include "UnitTest/CaseMatrix44.h"
#include "UnitTest/CaseSmallMap.h"
#include "UnitTest/CaseQuaternion.h"
#include "UnitTest/CaseTransform.h"
#include "UnitTest/CaseWinding.h"
#include "UnitTest/CasePath.h"
#include "UnitTest/CaseXmlPullParser.h"
#include "UnitTest/CaseXmlDocument.h"
#include "UnitTest/CaseZip.h"
#include "UnitTest/CaseSpatialHashArray.h"
#include "UnitTest/CaseRenderSw.h"
#include "UnitTest/CaseSound.h"
#include "Core/Log/Log.h"

using namespace traktor;

#if !defined(_WIN32) || defined(_CONSOLE)
int main(int /*argc*/, const char** /*argv*/)
#elif defined(WINCE)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{
	//log::info << L"Case cycle debugger" << Endl;
	//traktor::CaseCycleDebugger().run();

	//log::info << L"Case atomic" << Endl;
	//traktor::CaseAtomic().run();

	//log::info << L"Case meta" << Endl;
	//traktor::CaseMeta().run();

	//log::info << L"Case process" << Endl;
	//traktor::CaseProcess().run();

	//log::info << L"Case thread" << Endl;
	//traktor::CaseThread().run();

	//log::info << L"Case job" << Endl;
	//traktor::CaseJob().run();

	//log::info << L"Case heap" << Endl;
	//traktor::CaseHeap().run();

	//log::info << L"Case math" << Endl;
	//traktor::CaseMath().run();

	//log::info << L"Case matrix 3x3" << Endl;
	//traktor::CaseMatrix33().run();

	//log::info << L"Case matrix 4x4" << Endl;
	//traktor::CaseMatrix44().run();

	//log::info << L"Case quaternion" << Endl;
	//traktor::CaseQuaternion().run();

	log::info << L"Case small map" << Endl;
	traktor::CaseSmallMap().run();

	//log::info << L"Case transform" << Endl;
	//traktor::CaseTransform().run();

	//log::info << L"Case winding" << Endl;
	//traktor::CaseWinding().run();

	//log::info << L"Case path" << Endl;
	//traktor::CasePath().run();

	//log::info << L"Case xml pull parser" << Endl;
	//traktor::CaseXmlPullParser().run();

	//log::info << L"Case xml document" << Endl;
	//traktor::CaseXmlDocument().run();

	//log::info << L"Case zip" << Endl;
	//traktor::CaseZip().run();

	//log::info << L"Case spatial hash array" << Endl;
	//traktor::CaseSpatialHashArray().run();

	//log::info << L"Case software renderer" << Endl;
	//traktor::CaseRenderSw().run();

	//log::info << L"Case sound" << Endl;
	//traktor::CaseSound().run();

	log::info << L"Finished" << Endl;
}
