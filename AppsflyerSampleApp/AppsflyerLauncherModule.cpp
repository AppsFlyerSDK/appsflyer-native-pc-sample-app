#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "AppsflyerLauncherModule.h"
#include "AppsflyerModule.cpp"

using namespace std;

CAppsflyerLauncherModule *AppsflyerLauncherModule()
{
	static CAppsflyerLauncherModule inv;
	return &inv;
}

CAppsflyerLauncherModule::CAppsflyerLauncherModule()
{
}

void CAppsflyerLauncherModule::Init(const char *dkey, const char *appid)
{
	devkey = dkey;
	appID = appid;
}

void CAppsflyerLauncherModule::Start(bool skipFirst)
{
	AppsflyerModule afc(devkey, appID);

	// app build id
	std::string app_version = "1.0.0";

	// create timestamp
	std::time_t t = std::time(0);
	std::ostringstream oss;
	oss << t;
	std::string timestamp = oss.str();

	RequestData req;
	req.timestamp = timestamp;
	req.device_os_version = "1.0.0";
	req.app_version = app_version;
	req.device_model = afc.get_OS();
	req.limit_ad_tracking = "false";
	req.request_id = afc.uuid_gen().c_str();

	// adding AF id to the request
	DeviceIDs af_id;
	af_id.type = "custom";
	af_id.value = afc.get_AF_id().c_str();
	req.device_ids.insert(req.device_ids.end(), af_id);

	tuple<CURLcode, long, int> tpl = afc.af_firstOpen_init(req);
	CURLcode res = std::get<CURLcode>(tpl);
	long rescode = std::get<long>(tpl);
	int context = std::get<int>(tpl);
	// auto [res, rescode, context] = afc.af_firstOpen_init(req);
	AppsflyerLauncherModule()->OnHTTPCallBack(res, rescode, context);
}

void CAppsflyerLauncherModule::LogEvent(std::string event_name, json event_parameters)
{
	AppsflyerModule afc(devkey, appID);

	// app build id
	std::string app_version = "1.0.0";

	// create timestamp
	std::time_t t = std::time(0);
	std::ostringstream oss;
	oss << t;
	std::string timestamp = oss.str();

	RequestData req;
	req.timestamp = timestamp;
	req.device_os_version = "1.0.0";
	req.app_version = app_version;
	req.device_model = afc.get_OS();
	req.limit_ad_tracking = "false";
	req.request_id = afc.uuid_gen().c_str();

	// adding AF id to the request
	DeviceIDs af_id;
	af_id.type = "custom";
	af_id.value = afc.get_AF_id().c_str();
	req.device_ids.insert(req.device_ids.end(), af_id);

	req.event_name = event_name;
	req.event_parameters = event_parameters;

	tuple<CURLcode, long, int> tpl = afc.af_inappEvent(req);
	CURLcode res = std::get<CURLcode>(tpl);
	long rescode = std::get<long>(tpl);
	int context = std::get<int>(tpl);
	// auto [res, rescode, context] = afc.af_inappEvent(req);
	AppsflyerLauncherModule()->OnHTTPCallBack(res, rescode, context);
}

void CAppsflyerLauncherModule::OnHTTPCallBack(CURLcode res, long responseCode, int context)
{
	if (res != CURLE_OK)
	{
		// response failed
		OnCallbackFailure(responseCode, context);
	}
	else
	{
		OnCallbackSuccess(responseCode, context);
		AppsflyerModule afc(devkey, appID);

		switch (context)
		{
		case FIRST_OPEN_REQUEST:
		case SESSION_REQUEST:
			if (responseCode == 202)
			{
				afc.increase_AF_counter();
			}
			break;
		case INAPP_EVENT_REQUEST:
			break;
		default:
			break;
		}
	}
}

void CAppsflyerLauncherModule::OnCallbackSuccess(long responseCode, int context)
{
	// Handle Success
	switch (context)
	{
	case FIRST_OPEN_REQUEST:
	case SESSION_REQUEST:
		// ** handle success for these callback **
		break;
	case INAPP_EVENT_REQUEST:
		// ** handle success for this callback **
		break;
	default:
		break;
	}
}

void CAppsflyerLauncherModule::OnCallbackFailure(long responseCode, int context)
{
	// Handle Failure
	switch (context)
	{
	case FIRST_OPEN_REQUEST:
	case SESSION_REQUEST:
		// ** handle failure for these callback **
		break;
	case INAPP_EVENT_REQUEST:
		// ** handle failure for this callback **
		break;
	default:
		break;
	}
}

std::string CAppsflyerLauncherModule::GetAppsFlyerUID()
{
	AppsflyerModule afc(devkey, appID);
	return afc.get_AF_id();
}

bool CAppsflyerLauncherModule::IsInstallOlderThanDate(std::string datestring)
{
	AppsflyerModule afc(devkey, appID);
	return afc.isInstallOlderThanDate(datestring);
}
