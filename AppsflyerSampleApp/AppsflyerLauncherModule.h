#pragma once
#include "RequestData.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <codecvt> // codecvt_utf8
#include <locale>  // wstring_convert

using json = nlohmann::json;

class CAppsflyerLauncherModule
{
public:
	// This method receives your api key and app id,
	//  This method receives your api key and app id,
	//  and initializes the AppsFlyer Connector
	void Init(const char *devkey, const char *appID);
	// sends "first open/session" request to AppsFlyer.
	void Start(bool skipFirst = false);
	void Stop();
	/*  These methods are called upon a un/successful http request callback.
	Those are placeholders that you can fill with the desired actions upon success/failure
	(within AppsflyerLauncherModule.cpp file) */
	void OnCallbackSuccess(long responseCode, int context);
	void OnCallbackFailure(long responseCode, int context);
	// This method receives an event name and json object and sends an in-app event to AppsFlyer.
	void LogEvent(std::string event_name, json event_parameters, json event_custom_parameters = {});
	void SetCustomerUserId(std::string customerUserID);
	// returns true whether the game was installed before the given date
	// bool isInstallOlderThanDate(std::string datestring);
	std::string GetAppsFlyerUID();
	bool IsInstallOlderThanDate(std::string datestring);
	std::string to_utf8(std::wstring& wide_string);
private:
	const char *devkey;
	const char *appID;
	bool isStopped;
	std::string cuid;
	friend CAppsflyerLauncherModule *AppsflyerLauncherModule();
	RequestData CreateRequestData();
	CAppsflyerLauncherModule();
	void OnHTTPCallBack(CURLcode res, long responseCode, int context);
};

const int FIRST_OPEN_REQUEST = 100;
const int SESSION_REQUEST = 101;
const int INAPP_EVENT_REQUEST = 102;

CAppsflyerLauncherModule *AppsflyerLauncherModule();