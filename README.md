---
title: Native PC C++
category: 6446526dddf659006c7ea807
order: 11
hidden: false
slug: nativepc-vanilla
---

# AppsFlyer Windows Launcher Integration

> 🚧 OS-Restriction
>
> This sample code currently supports windows environment only.

## **Getting started with AppsFlyer Integration**

AppsFlyer empowers marketers and helps them make better decisions.

This is done by providing marketers with powerful tools that solve real pain points. These include cross-platform attribution, mobile and web analytics, deep linking, fraud detection, privacy management and preservation, and much more.

With this sample app, we will be able to demonstrate basic integration which includes the first open/sessions and in-app events (i.e purchase events).

AppsFlyer requires the game to report activities within it like app open. In order to do that, the app communicate with the AppsFlyer APIs over HTTPS - the sample app includes the code that does that.
you may use this sample app as a reference for integrating AppsFlyer into your C++ app.

<hr/>

### Pre-requisite

[vcpkg](https://vcpkg.io/en/index.html) - openssl & nlohmann-json packages

```
vcpkg install nlohmann-json:x86-windows
vcpkg install openssl:x86-windows</code></pre>
```

<hr/>

## **AppsflyerLauncherModule - Interface**

AppsflyerLauncherModule.h”, which is include in the folder copied above, contains the required code and logic to connect to our servers and report events.

<br/>

#### void start(const char* devkey, const char* appID)

This method receives your api key and app id, and initializes the AppsFlyer Module (and sends “first open/session” request to AppsFlyer).

##### <span style="text-decoration:underline;">Usage:</span>

```
AppsflyerLauncherModule()->start("DEV_KEY", "APP_ID");
</code></pre>

```

##### App-Details

- DEV_KEY - retrieve the Dev key from the marketer or the [AppsFlyer HQ](https://support.appsflyer.com/hc/en-us/articles/211719806-App-settings-#general-app-settings).
- APP_ID

<br/>

#### void **logEvent**(std::string **event_name**, json **event_values**)

This method receives an event name and json object and sends an in-app event to AppsFlyer.

##### <span style="text-decoration:underline;">Usage:</span>

```
json event_values = { {"af_currency", "USD"}, {"af_price", 6.66}, {"af_revenue", 24.12} };
std::string event_name = "af_purchase";
AppsflyerLauncherModule()->logEvent(event_name, event_values);
```

##### \*In order to use json please make sure to use the following imports:

```
#include &lt;nlohmann/json.hpp>
using json = nlohmann::json;
```

<hr>

## Running the Sample App

1. Install [Visual Studio](https://visualstudio.microsoft.com/)
2. Open the solution
3. Open "AppsflyerSampleApp.cpp" file
4. On line 112, replace DEV_KEY and APP_ID with your [app details](#App-Details)
5. Run the app by pressing on the play button on the top toolbar ("Local Windows Debugger"). Make sure that the mode is set to Debug.
6. After 24 hours, the dashboard will update and show organic/non-organic install and in-app events.

<hr/>

## **Implementing AppsFlyer into your own C++ app**

### Set Up

1. Copy the files from the "appsflyer-module" folder into your C++ project under Header Files -> AppsFlyer
2. Import the Module:

```
#include "AppsflyerLauncherModule.h"
```

3. Import nlohmann-json

```
#include &lt;nlohmann/json.hpp>
using json = nlohmann::json;
```

5. [Initialize](#void-startconst-char-devkey-const-char-appid) the AppsFlyer integration
6. Report [in-app events](#void-logeventstdstring-event_name-json-event_values)
