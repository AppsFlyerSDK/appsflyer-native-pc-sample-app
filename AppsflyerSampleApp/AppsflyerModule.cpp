
#include <curl/curl.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "AppsflyerLauncherModule.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <string>
#include <sstream>

using namespace std;

class AppsflyerModule
{
public:
	AppsflyerModule(const char *devkey, std::string appid)
	{
		_devkey = devkey;
		_appid = appid;
	}

	// send curl with hmac auth and json data
	tuple<CURLcode, long> send_http_post(std::string &url, std::string jsonData, int ulContextValue)
	{
		CURL *curl;
		CURLcode res{};
		long response_code;
		std::string readBuffer;
		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

			char *key = _strdup(_devkey);
			int keylen = strlen(key);
			const unsigned char *data = (const unsigned char *)_strdup(jsonData.c_str());

			int datalen = strlen((char *)data);
			unsigned char *result = NULL;
			unsigned int resultlen = -1;

			result = mx_hmac_sha256((const void *)key, keylen, data, datalen, result, &resultlen);

			for (unsigned int i = 0; i < resultlen; i++)
			{
				// printf2("%02hhX", result[i]); // or just "%02X" if you are not using C11 or later
			}

			// allocate memory for array
			std::stringstream ss;
			for (unsigned int i = 0; i < resultlen; i++)
			{
				ss << hex << setw(2) << setfill('0') << (int)static_cast<unsigned char>(result[i]);
			}
			std::string mystr = ss.str();

			const char *a = "Authorization: ";
			const char *b = mystr.c_str();

			char buf[100];
			strcpy_s(buf, a);
			strcat_s(buf, b);

			struct curl_slist *slist = NULL;
			slist = curl_slist_append(slist, "Content-Type: application/json");
			slist = curl_slist_append(slist, buf);
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); // for --insecure option
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonData.length());
			curl_easy_setopt(curl, CURLOPT_POST, 1);
			std::string userAgentStr = "Native PC/HTTP Client 1.0 (" + _appid + ")";
			const char *userAgent = userAgentStr.c_str();
			curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent);
			// curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888"); // redirect traffic to Fiddler for debugging

			/* Perform the request, res will get the return code */
			curl_easy_perform(curl);

			/* Check for errors */
			if (res != CURLE_OK)
			{
				response_code = 0;
				fprintf(stderr, "curl_easy_perform() failed: %s\n",
						curl_easy_strerror(res));
			}
			else
			{
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
				// std::string test = readBuffer;
			}
			/* always cleanup */
			curl_easy_cleanup(curl);
		}
		return {res, response_code};
	}

	// report first open event to AppsFlyer (or session if counter > 2)
	tuple<CURLcode, long, int> af_firstOpen_init(RequestData req)
	{
		// send requests
		int af_counter = get_AF_counter();
		if (af_counter < 2)
		{
			// auto [res, rescode] = af_firstOpenRequest(req);
			tuple<CURLcode, long> tpl = af_firstOpenRequest(req);
			CURLcode res = std::get<CURLcode>(tpl);
			long rescode = std::get<long>(tpl);
			return {res, rescode, FIRST_OPEN_REQUEST};
		}
		else
		{
			// auto [res, rescode] = af_sessionRequest(req);
			tuple<CURLcode, long> tpl = af_sessionRequest(req);
			CURLcode res = std::get<CURLcode>(tpl);
			long rescode = std::get<long>(tpl);
			return {res, rescode, SESSION_REQUEST};
		}
	}

	// report inapp event to AppsFlyer
	tuple<CURLcode, long, int> af_inappEvent(RequestData req)
	{
		std::string url = "https://events.appsflyer.com/v1.0/c2s/inapp/app/nativepc/" + _appid;

		/* Now specify the POST data */
		std::ostringstream oss;

		// use ADL to select best to_string function
		std::string jsonData = postDataStr(req, true);

		// auto [res, rescode] = send_http_post(url, jsonData, INAPP_EVENT_REQUEST);
		tuple<CURLcode, long> tpl = send_http_post(url, jsonData, INAPP_EVENT_REQUEST);
		CURLcode res = std::get<CURLcode>(tpl);
		long rescode = std::get<long>(tpl);
		return {res, rescode, INAPP_EVENT_REQUEST};
	}

	// return af uuid
	std::string get_AF_id()
	{
		std::string af_id = reg_getKey(reg_key, reg_path, "AF_uuid");
		if (af_id.empty())
		{
			af_id = uuid_gen();
			reg_writeKey(reg_key, reg_path, "AF_uuid", af_id);
			return af_id;
		}
		return af_id;
	}

	// increase the AF open counter in the registry
	void increase_AF_counter()
	{
		std::string new_counter = std::to_string(get_AF_counter() + 1);
		reg_writeKey(reg_key, reg_path, "AF_counter", new_counter);
	}

	// get the OS type
	std::string get_OS()
	{
#ifdef _WIN32
		return ("Windows_32-bit");
#elif _WIN64
		return ("Windows_64-bit");
#elif __APPLE__ || __MACH__
		return ("Mac_OSX");
#elif __linux__
		return ("Linux");
#elif __FreeBSD__
		return ("FreeBSD");
#elif __unix || __unix__
		return ("Unix");
#else
		return ("Other");
#endif
	}

	// generate UUID
	std::string uuid_gen()
	{
		GUID gidReference;
		HRESULT hCreateGuid = CoCreateGuid(&gidReference);
		return GUID_toString(&gidReference);
	}

	std::time_t to_time_t(const std::string &str, bool is_dst = false, const std::string &format = "%Y-%b-%d %H:%M:%S")
	{
		std::tm t = {0};
		t.tm_isdst = is_dst ? 1 : 0;
		std::istringstream ss(str);
		ss >> std::get_time(&t, format.c_str());
		return mktime(&t);
	}

	bool isInstallOlderThanDate(string date)
	{
		bool isInstallOlder = false;
		TCHAR szPath[MAX_PATH];
		GetModuleFileName(NULL, szPath, MAX_PATH);

		struct stat result;
		if (stat(szPath, &result) == 0)
		{
			__time64_t mod_time = result.st_mtime;
			std::time_t excludeInstallDateBefore = to_time_t(date);
			double diff = difftime(mod_time, excludeInstallDateBefore);
			isInstallOlder = diff < 0;
		}

		return isInstallOlder;
	}

private:
	// registry key to save the AF data.
	HKEY reg_key = HKEY_CURRENT_USER;

	std::string _appid;

	// the AF app _devkey
	const char *_devkey;

	// the registry path for saving the AF data.
	std::string reg_path = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";

	// convert guid to string
	std::string GUID_toString(GUID *guid)
	{
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
	std::string reg_getKey(HKEY hkey, const std::string &location, const std::string &name)
	{
		HKEY key;
		TCHAR value[1024];
		DWORD bufLen = 1024 * sizeof(TCHAR);
		long ret;
		ret = RegOpenKeyExA(hkey, location.c_str(), 0, KEY_QUERY_VALUE, &key);
		if (ret != ERROR_SUCCESS)
		{
			return std::string();
		}
		std::string fullname = name + "_" + _appid;
		ret = RegQueryValueExA(key, fullname.c_str(), 0, 0, (LPBYTE)value, &bufLen);
		RegCloseKey(key);
		if ((ret != ERROR_SUCCESS) || (bufLen > 1024 * sizeof(TCHAR)))
		{
			return std::string();
		}
		std::string stringValue = std::string(value, (size_t)bufLen - 1);
		size_t i = stringValue.length();
		while (i > 0 && stringValue[i - 1] == '\0')
		{
			--i;
		}
		return stringValue.substr(0, i);
	}

	// save data to registry by key
	void reg_writeKey(HKEY hkey, const std::string &location, const std::string &name, const std::string &data)
	{

		HKEY key_handle;
		LONG result = RegOpenKeyExA(hkey, location.c_str(), 0, KEY_WRITE, &key_handle);

		if (ERROR_SUCCESS == result)
		{
			std::string fullname = name + "_" + _appid;
			result = RegSetValueExA(key_handle, (fullname.c_str()),
									0,
									REG_SZ,
									(unsigned char *)data.c_str(),
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
	int get_AF_counter()
	{
		std::string counter = reg_getKey(reg_key, reg_path, "AF_counter");
		if (counter.empty())
		{
			counter = "0";
			reg_writeKey(reg_key, reg_path, "AF_counter", counter);
			return std::stoi(counter);
		}
		return std::stoi(counter);
	}

	std::string postDataStr(RequestData req, bool isEvent = false) {
		std::ostringstream oss;
		oss << "{\"device_ids\":[{\"type\":\"" << req.device_ids[0].type << "\",\"value\":\"" << req.device_ids[0].value << "\"}";
		oss << "],\"request_id\":\"" << req.request_id << "\",\"device_os_version\":\"" << req.device_os_version << "\",\"device_model\":\"" << req.device_model << "\",\"limit_ad_tracking\":" << req.limit_ad_tracking << ",\"app_version\":\"" << req.app_version << "\"";
		if (isEvent) {
			oss << ",\"event_parameters\":" << req.event_parameters << ",\"event_name\":\"" << req.event_name << "\"";
			if (!req.event_custom_parameters.is_null()) {
				oss << ",\"event_custom_parameters\":" << req.event_custom_parameters;
			}
		}
		if (!req.customer_user_id.empty()) {
			oss << ",\"customer_user_id\":\"" << req.customer_user_id << "\"";
		}
		oss << "}";

		return oss.str();
	}

	// report first open event to AppsFlyer
	tuple<CURLcode, long> af_firstOpenRequest(RequestData req)
	{
		std::string url = "https://events.appsflyer.com/v1.0/c2s/first_open/app/nativepc/" + _appid;

		/* Now specify the POST data */
		std::string jsonData = postDataStr(req);

		return send_http_post(url, jsonData, FIRST_OPEN_REQUEST);
		// CURLcode res = send_http_post(url, jsonData);
	}

	// report session event (after the counter passes 2 opens) to AppsFlyer
	tuple<CURLcode, long> af_sessionRequest(RequestData req)
	{
		std::string url = "https://events.appsflyer.com/v1.0/c2s/session/app/nativepc/" + _appid;

		/* Now specify the POST data */
		std::string jsonData = postDataStr(req);

		return send_http_post(url, jsonData, SESSION_REQUEST);
	}

	// encrypt data with _devkey
	unsigned char *mx_hmac_sha256(const void *key, int keylen, const unsigned char *data, int datalen,
								  unsigned char *result, unsigned int *resultlen)
	{
		return HMAC(EVP_sha256(), key, keylen, data, datalen, result, resultlen);
	}
};
