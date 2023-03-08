
#include <curl/curl.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "AppsflyerLauncherModule.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

class AppsflyerModule {
public:

	AppsflyerModule(const char* devkey, std::string appid) {
		_devkey = devkey;
		_appid = appid;
	}

	//send curl with hmac auth and json data
	void send_http_post(std::string& url, std::string jsonData, int ulContextValue) {
		CURL* curl;
		CURLcode res;
		std::string readBuffer;
		curl = curl_easy_init();
		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

			char* key = _strdup(_devkey);
			int keylen = strlen(key);
			const unsigned char* data = (const unsigned char*)_strdup(jsonData.c_str());

			int datalen = strlen((char*)data);
			unsigned char* result = NULL;
			unsigned int resultlen = -1;

			result = mx_hmac_sha256((const void*)key, keylen, data, datalen, result, &resultlen);

			for (unsigned int i = 0; i < resultlen; i++) {
				//printf2("%02hhX", result[i]); // or just "%02X" if you are not using C11 or later
			}

			//allocate memory for array
			std::stringstream ss;
			for (unsigned int i = 0; i < resultlen; i++) {
				ss << hex << setw(2) << setfill('0') << (int) static_cast <unsigned char>(result[i]);
			}
			std::string mystr = ss.str();

			const char* a = "Authorization: ";
			const char* b = mystr.c_str();

			char buf[100];
			strcpy_s(buf, a);
			strcat_s(buf, b);

			struct curl_slist* slist = NULL;
			slist = curl_slist_append(slist, "Content-Type: application/json");
			slist = curl_slist_append(slist, buf);
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);  // for --insecure option
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonData.length());
			curl_easy_setopt(curl, CURLOPT_POST, 1);
			//curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888"); // redirect traffic to Fiddler for debugging
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

			/* Perform the request, res will get the return code */
			res = curl_easy_perform(curl);
			/* Check for errors */
			if (res != CURLE_OK) {
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
			}

			long response_code;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

			switch (ulContextValue)
			{
				case FIRST_OPEN_REQUEST:
				case SESSION_REQUEST:
					if (response_code == 202)
					{
						increase_AF_counter();
					}
					break;
				case INAPP_EVENT_REQUEST:
					break;
				default:
					break;
			}

			std::string test = readBuffer;

			/* always cleanup */
			curl_easy_cleanup(curl);

		}
	}

	// report first open event to AppsFlyer (or session if counter > 2)
	void af_firstOpen_init(RequestData req) {
		//send requests
		int af_counter = get_AF_counter();
		if (af_counter < 2) {
			return af_firstOpenRequest(req);
		}
		else {
			return af_sessionRequest(req);
		}
	}

	// report inapp event to AppsFlyer 
	void af_inappEvent(RequestData req) {
		std::string url = "https://events.appsflyer.com/v1.0/c2s/inapp/app/webos/" + _appid;

		/* Now specify the POST data */
		std::ostringstream oss;

		// use ADL to select best to_string function
		auto event_values_j_str = to_string(req.event_values);  // calling nlohmann::to_string

		oss << "{\"device_ids\":[{\"type\":\"" << req.device_ids[0].type << "\",\"value\":\"" << req.device_ids[0].value << "\"}],\"request_id\":\"" << req.request_id << "\",\"device_os_version\":\"" << req.device_os_version << "\",\"device_model\":\"" << req.device_model << "\",\"limit_ad_tracking\":" << req.limit_ad_tracking << ",\"app_version\":\"" << req.app_version << "\",\"event_parameters\":" << event_values_j_str << ",\"event_name\":\"" << req.event_name << "\"}";
		std::string jsonData = oss.str();

		return send_http_post(url, jsonData, INAPP_EVENT_REQUEST);
	}

	// return af uuid
	std::string get_AF_id() {
		std::string af_id = reg_getKey(reg_key, reg_path, "AF_uuid");
		if (af_id.empty()) {
			af_id = uuid_gen();
			reg_writeKey(reg_key, reg_path, "AF_uuid", af_id);
			return af_id;
		}
		return af_id;
	}

	// increase the AF open counter in the registry
	void increase_AF_counter() {
		std::string new_counter = std::to_string(get_AF_counter() + 1);
		reg_writeKey(reg_key, reg_path, "AF_counter", new_counter);
	}

	// get the OS type
	std::string get_OS() {
#ifdef _WIN32
		return("Windows_32-bit");
#elif _WIN64
		return("Windows_64-bit");
#elif __APPLE__ || __MACH__
		return("Mac_OSX");
#elif __linux__
		return("Linux");
#elif __FreeBSD__
		return("FreeBSD");
#elif __unix || __unix__
		return("Unix");
#else
		return("Other");
#endif
	}

	// generate UUID
	std::string uuid_gen() {
		GUID gidReference;
		HRESULT hCreateGuid = CoCreateGuid(&gidReference);
		return GUID_toString(&gidReference);
	}
private:
	// registry key to save the AF data.
	HKEY reg_key = HKEY_CURRENT_USER;

	std::string _appid;

	// the AF app _devkey
	const char* _devkey;

	// the registry path for saving the AF data.
	std::string reg_path = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

	// convert guid to string
	std::string GUID_toString(GUID* guid) {
		char guid_string[37]; // 32 hex chars + 4 hyphens + null terminator
		snprintf(
			guid_string, sizeof(guid_string),
			"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			guid->Data1, guid->Data2, guid->Data3,
			guid->Data4[0], guid->Data4[1], guid->Data4[2],
			guid->Data4[3], guid->Data4[4], guid->Data4[5],
			guid->Data4[6], guid->Data4[7]);
		return guid_string;
	}

	// retreive data from registry by key
	std::string reg_getKey(HKEY hkey, const std::string& location, const std::string& name) {
		HKEY key;
		TCHAR value[1024];
		DWORD bufLen = 1024 * sizeof(TCHAR);
		long ret;
		ret = RegOpenKeyExA(hkey, location.c_str(), 0, KEY_QUERY_VALUE, &key);
		if (ret != ERROR_SUCCESS) {
			return std::string();
		}
		std::string fullname = name + "_" + _appid;
		ret = RegQueryValueExA(key, fullname.c_str(), 0, 0, (LPBYTE)value, &bufLen);
		RegCloseKey(key);
		if ((ret != ERROR_SUCCESS) || (bufLen > 1024 * sizeof(TCHAR))) {
			return std::string();
		}
		std::string stringValue = std::string(value, (size_t)bufLen - 1);
		size_t i = stringValue.length();
		while (i > 0 && stringValue[i - 1] == '\0') {
			--i;
		}
		return stringValue.substr(0, i);
	}

	// save data to registry by key
	void reg_writeKey(HKEY hkey, const std::string& location, const std::string& name, const std::string& data) {

		HKEY key_handle;
		LONG result = RegOpenKeyExA(hkey, location.c_str(), 0, KEY_WRITE, &key_handle);

		if (ERROR_SUCCESS == result)
		{
			std::string fullname = name + "_" + _appid;
			result = RegSetValueExA(key_handle, (fullname.c_str()),
				0,
				REG_SZ,
				(unsigned char*)data.c_str(),
				data.length() * sizeof(wchar_t));

			if (result != ERROR_SUCCESS)
			{
				printf("Error setting key %d\n", GetLastError());
			}
		}
		else
		{
			printf("Error opening key %d\n", GetLastError());
		}
	}

	// get the AF app open counter from registry
	int get_AF_counter() {
		std::string counter = reg_getKey(reg_key, reg_path, "AF_counter");
		if (counter.empty()) {
			counter = "0";
			reg_writeKey(reg_key, reg_path, "AF_counter", counter);
			return std::stoi(counter);
		}
		return std::stoi(counter);
	}

	// report first open event to AppsFlyer 
	void af_firstOpenRequest(RequestData req) {
		std::string url = "https://events.appsflyer.com/v1.0/c2s/first_open/app/webos/" + _appid;

		/* Now specify the POST data */
		std::ostringstream oss;
		oss << "{\"device_ids\":[{\"type\":\"" << req.device_ids[0].type << "\",\"value\":\"" << req.device_ids[0].value.c_str() << "\"}],\"timestamp\":" << req.timestamp << ",\"request_id\":\"" << req.request_id << "\",\"device_os_version\":\"" << req.device_os_version << "\",\"device_model\":\"" << req.device_model << "\",\"limit_ad_tracking\":" << req.limit_ad_tracking << ",\"app_version\":\"" << req.app_version << "\"}";
		std::string jsonData = oss.str();

		return send_http_post(url, jsonData, FIRST_OPEN_REQUEST);
		//CURLcode res = send_http_post(url, jsonData);
	}

	// report session event (after the counter passes 2 opens) to AppsFlyer 
	void af_sessionRequest(RequestData req) {
		std::string url = "https://events.appsflyer.com/v1.0/c2s/session/app/webos/" + _appid;

		/* Now specify the POST data */
		std::ostringstream oss;
		oss << "{\"device_ids\":[{\"type\":\"" << req.device_ids[0].type << "\",\"value\":\"" << req.device_ids[0].value.c_str() << "\"}],\"timestamp\":" << req.timestamp << ",\"request_id\":\"" << req.request_id << "\",\"device_os_version\":\"" << req.device_os_version << "\",\"device_model\":\"" << req.device_model << "\",\"limit_ad_tracking\":" << req.limit_ad_tracking << ",\"app_version\":\"" << req.app_version << "\"}";
		std::string jsonData = oss.str();

		return send_http_post(url, jsonData, SESSION_REQUEST);
	}

	// encrypt data with _devkey
	unsigned char* mx_hmac_sha256(const void* key, int keylen, const unsigned char* data, int datalen,
		unsigned char* result, unsigned int* resultlen) {
		return HMAC(EVP_sha256(), key, keylen, data, datalen, result, resultlen);
	}
		};
