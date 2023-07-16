---
title: Native PC C++
parentDoc: 64ad6456ceede10cf0b2a120
category: 6446526dddf659006c7ea807
order: 3
hidden: false
slug: nativepc-vanilla
---

> Link to repository  
> [GitHub](https://github.com/AppsFlyerSDK/appsflyer-native-pc-sample-app)

## AppsFlyer Native PC C++ SDK integration

AppsFlyer empowers gaming marketers to make better decisions by providing powerful tools that solve real pain points, including cross-platform attribution, mobile and web analytics, deep linking, fraud detection, privacy management and preservation, and more.

Game attribution requires the game to communicate with AppsFlyer APIs over HTTPS and report user activities like first opens, consecutive sessions, and in-app events. For example, purchase events.
We recommend you use this sample app as a reference for integrating the code that reports user activities in your C++. **Note**: The sample code that follows is currently only supported in a Windows environment.

### Prerequisites

[vcpkg](https://vcpkg.io/en/index.html) openssl & nlohmann-json packages:

```c++
vcpkg install curl:x86-windows
vcpkg install nlohmann-json:x86-windows
vcpkg install openssl:x86-windows
```

<hr/>

## AppsflyerLauncherModule - Interface

`AppsflyerLauncherModule.h`, included in the `appsflyer-module` folder, contains the required code and logic to connect to AppsFlyer servers and report events.

### Init

This method receives your API key and app ID and initializes the AppsFlyer Module.

**Method signature**

```c++
void Init(const char* devkey, const char* appID)
```

**Usage**:

```c++
AppsflyerLauncherModule()->Init(<< DEV_KEY >>, << APP_ID >>);
```

<span id="app-details">**Arguments**:</span>

- `APP_ID`, `DEV_KEY`: Get from the marketer or [AppsFlyer HQ](https://support.appsflyer.com/hc/en-us/articles/211719806-App-settings-#general-app-settings).

### Start

This method sends first open/session requests to AppsFlyer.

**Method signature**

```c++
void Start(bool skipFirst = false)
```

**Usage**:

```c++
// without the flag
AppsflyerLauncherModule()->Start();

// with the flag
bool skipFirst = [SOME_CONDITION];
AppsflyerLauncherModule()->Start(skipFirst);
```

### LogEvent

This method receives an event name and JSON object and sends in-app events to AppsFlyer.

**Method signature**

```c++
void LogEvent(std::string event_name, json event_parameters)
```

**Usage**:

```c++
json event_parameters = { {"af_currency", "USD"}, {"af_price", 6.66}, {"af_revenue", 24.12} };
std::string event_name = "af_purchase";
AppsflyerLauncherModule()->LogEvent(event_name, event_parameters);
```

**Note**: To use the JSON, make sure to use the following imports:

```c++
#include <nlohmann/json.hpp>
using json = nlohmann::json;
```

### OnCallbackSuccess, OnCallbackFailure

The above methods are placeholders for the desired actions upon success/failure.  
It is possible to handle different types of events with the switch case of the context within each function (“FIRST_OPEN_REQUEST”, ”SESSION_REQUEST”, ”INAPP_EVENT_REQUEST”).

**Method signature**

```c++
void OnCallbackSuccess(long responseCode, uint64 context)
void OnCallbackFailure(long responseCode, uint64 context)
```

### GetAppsFlyerUID

Get AppsFlyer's unique device ID. The SDK generates an AppsFlyer unique device ID upon app installation. When the SDK is started, this ID is recorded as the ID of the first app install.

**Method signature**

```c++
std::string GetAppsFlyerUID()
```

**Usage**:

```c++
AppsflyerLauncherModule()->GetAppsFlyerUID();
```

### IsInstallOlderThanDate

This method receives a date string and returns true if the game exe modification date is older than the date string. The date string format is: "2023-January-01 23:12:34"

**Method signature**

```c++
bool IsInstallOlderThanDate(std::string datestring)
```

**Usage**:

```c++
// the modification date in this example is "2023-January-23 08:30:00"

// will return false
bool dateBefore = AppsflyerLauncherModule()->IsInstallOlderThanDate("2023-January-01 23:12:34");

// will return true
bool dateAfter = AppsflyerLauncherModule()->IsInstallOlderThanDate("2023-April-10 23:12:34");

// example usage with skipFirst -
// skipping if the install date is NOT older than the given date
bool isInstallOlderThanDate = AppsflyerLauncherModule()->IsInstallOlderThanDate("2023-January-10 23:12:34");
AppsflyerLauncherModule()->Start(!isInstallOlderThanDate);
```

## Running the sample app

1. Install [Visual Studio](https://visualstudio.microsoft.com/).
2. Open the solution.
3. Open the `AppsflyerSampleApp.cpp` file.
4. On line 110, replace `DEV_KEY` and `APP_ID` with your [app details](#app-details).
5. Run the app by clicking **Play** on the top toolbar (Local Windows Debugger). Make sure that the mode is set to Debug.
6. After 24 hours, the dashboard updates and shows organic and non-organic installs and in-app events.

## Implementing AppsFlyer in your C++ app

### Setup

1. Copy the files from the `appsflyer-module` folder into your C++ project under **Header Files** > **AppsFlyer**.
2. Import the Module:

```c++
#include "AppsflyerLauncherModule.h"
```

3. Import `nlohmann-json`:

```c++
#include <nlohmann/json.hpp>
using json = nlohmann::json;
```

4. [Initialize](#init) and [start](#start) the AppsFlyer integration.
5. Report [in-app events](#logevent).

## Resetting the attribution

Remove the registry data from `SOFTWARE\Microsoft\Windows\CurrentVersion\Run`. The Registry keys are `AF_counter_[APPID]` and `AF_uuid_[APPID]`.
