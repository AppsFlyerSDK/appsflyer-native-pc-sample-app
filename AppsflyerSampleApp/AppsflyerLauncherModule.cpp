#include <curl/curl.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "AppsflyerLauncherModule.h"
#include "AppsflyerModule.cpp"

using namespace std;


CAppsflyerLauncherModule* AppsflyerLauncherModule()
{
	static CAppsflyerLauncherModule inv;
	return &inv;
}

CAppsflyerLauncherModule::CAppsflyerLauncherModule() { }

void CAppsflyerLauncherModule::start(const char * dkey, const char * appid) {
	// testing af_firstOpen/af_session and af_inappEvent 
	devkey = dkey;
	appID = appid;
	AppsflyerModule afc(devkey, appID);

	std::string app_version = "1.0.0";

	//create timestamp
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

	//adding AF id to the request
	DeviceIDs af_id;
	af_id.type = "custom";
	af_id.value = afc.get_AF_id().c_str();
	req.device_ids.insert(req.device_ids.end(), af_id);

	afc.af_firstOpen_init(req);
}

void CAppsflyerLauncherModule::logEvent(std::string event_name, json event_values) {
	AppsflyerModule afc(devkey, appID);

	std::string app_version = "1.0.0";

	//create timestamp
	std::time_t t = std::time(0);
	std::ostringstream oss;
	oss << t;
	std::string timestamp = oss.str();

	RequestData req;
	req.timestamp = timestamp;
	req.device_os_version = "1.0.0";
	req.app_version = app_version;
	req.device_model = afc.get_OS(); //TODO: check how to retreive device model - in the meantime send 'steam'
	req.limit_ad_tracking = "false";
	req.request_id = afc.uuid_gen().c_str();

	//adding AF id to the request
	DeviceIDs af_id;
	af_id.type = "custom";
	af_id.value = afc.get_AF_id().c_str();
	req.device_ids.insert(req.device_ids.end(), af_id);
	req.event_name = event_name;
	req.event_values = event_values;

	afc.af_inappEvent(req);
}
