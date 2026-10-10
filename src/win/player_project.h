// player_project.h
// Windows product identity. User-agent strings stay in project.h.
// Included only by src/win. The Linux build does not compile these files.

#ifndef PLAYER_PROJECT_H
#define PLAYER_PROJECT_H

#include "project.h"

#undef PROJECT_ID
#undef PROJECT_NAME
#undef PROJECT_DESCRIPTION
#undef PERSISTENCE_STORAGE_PATH

#define PROJECT_ID "gemini-player"
#define PROJECT_NAME "Gemini"
#define PROJECT_DESCRIPTION "A lightweight native Gemini player built with WebView2 on Windows."
#define PERSISTENCE_STORAGE_PATH ".app/" PROJECT_ID "/cookies"

/* UA_WEBVIEW names Ubuntu WebKit. This process is the Edge WebView2 runtime. */
#define PLAYER_DEFAULT_USER_AGENT UA_EDGE

#endif
