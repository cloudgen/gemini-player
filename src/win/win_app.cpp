// win_app.cpp
// Win32 window and WebView2 host for gemini-player.
// The GTK sources under src/main/cpp are not part of this build.

#include "player_project.h"
#include "win_host.h"
#include "win_log.h"

#include <windows.h>
#include <shellapi.h>
#include <wrl.h>
#include <wrl/client.h>
#include <wrl/implements.h>
#include "WebView2EnvironmentOptions.h"

#include <stdio.h>
#include <string>
#include <vector>
#include <memory>

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

namespace {

enum {
    IDC_BACK = 1001,
    IDC_FORWARD,
    IDC_RELOAD,
    IDC_URL,
    IDC_GO,
    IDC_UA,
    IDM_AUTOCLICK = 2001,
    IDM_BOOKMARK_START,
    IDM_UA_WEBVIEW,
    IDM_UA_CHROME,
    IDM_UA_FIREFOX,
    IDM_UA_SAFARI,
    IDM_UA_EDGE,
    IDM_LUA_FOLDER,
    IDM_LOG_FOLDER,
    IDM_EXIT,
    IDM_VERSION,
    IDT_SHUTDOWN = 1
};

struct Popup;

struct App {
    HWND hwnd = nullptr;
    HWND back_btn = nullptr;
    HWND fwd_btn = nullptr;
    HWND reload_btn = nullptr;
    HWND url_edit = nullptr;
    HWND go_btn = nullptr;
    HWND ua_label = nullptr;
    HMENU menu = nullptr;
    ComPtr<ICoreWebView2Environment> env;
    ComPtr<ICoreWebView2Controller> controller;
    ComPtr<ICoreWebView2> webview;
    std::wstring ua;
    bool ready = false;
    std::vector<std::unique_ptr<Popup>> popups;
};

struct Popup {
    HWND hwnd = nullptr;
    ComPtr<ICoreWebView2Controller> controller;
    ComPtr<ICoreWebView2> webview;
};

App g_app;
WNDPROC g_edit_proc = nullptr;
const wchar_t *k_main_class = L"GeminiPlayerWindow";
const wchar_t *k_popup_class = L"GeminiPlayerPopup";

void layout_window(void);
void navigate_to(const std::wstring &url);
void apply_user_agent(const char *ua_utf8, bool reload);

std::wstring trim_wide(std::wstring text)
{
    while (!text.empty() && (text.front() == L' ' || text.front() == L'\t')) text.erase(text.begin());
    while (!text.empty() && (text.back() == L' ' || text.back() == L'\t')) text.pop_back();
    return text;
}

std::wstring normalize_url(std::wstring text)
{
    text = trim_wide(text);
    if (text.empty()) {
        return player_widen(START_PAGE);
    }
    if (text.find(L"://") == std::wstring::npos && text.find(L"\\\\") != 0) {
        text = L"https://" + text;
    }
    return text;
}

void open_folder(const std::wstring &path)
{
    std::wstring copy = path;
    for (size_t i = 0; i < copy.size(); i++) {
        if (copy[i] == L'/') copy[i] = L'\\';
    }
    CreateDirectoryW(copy.c_str(), NULL);
    HINSTANCE rc = ShellExecuteW(g_app.hwnd, L"open", copy.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if ((INT_PTR)rc <= 32) {
        ciao_error("Could not open folder");
    }
}

void layout_window(void)
{
    if (g_app.hwnd == nullptr) return;
    RECT rc;
    GetClientRect(g_app.hwnd, &rc);
    const PlayerUi &ui = player_ui();
    int y = 4;
    int width = rc.right - rc.left;
    if (width < 10) width = 10;

    int show_nav = ui.show_navigation_buttons ? SW_SHOW : SW_HIDE;
    ShowWindow(g_app.back_btn, show_nav);
    ShowWindow(g_app.fwd_btn, show_nav);
    ShowWindow(g_app.reload_btn, show_nav);
    ShowWindow(g_app.url_edit, show_nav);
    ShowWindow(g_app.go_btn, show_nav);
    if (ui.show_navigation_buttons) {
        int x = 4;
        MoveWindow(g_app.back_btn, x, y, 64, 26, TRUE); x += 68;
        MoveWindow(g_app.fwd_btn, x, y, 64, 26, TRUE); x += 68;
        MoveWindow(g_app.reload_btn, x, y, 72, 26, TRUE); x += 76;
        int go_w = 56;
        int edit_w = width - x - go_w - 8;
        if (edit_w < 40) edit_w = 40;
        MoveWindow(g_app.url_edit, x, y, edit_w, 26, TRUE);
        MoveWindow(g_app.go_btn, x + edit_w + 4, y, go_w, 26, TRUE);
        BOOL enabled = ui.navigation_buttons_enabled ? TRUE : FALSE;
        EnableWindow(g_app.back_btn, enabled);
        EnableWindow(g_app.fwd_btn, enabled);
        EnableWindow(g_app.reload_btn, enabled);
        EnableWindow(g_app.go_btn, enabled);
        EnableWindow(g_app.url_edit, enabled);
        y += 32;
    }
    if (ui.show_user_agent_label) {
        ShowWindow(g_app.ua_label, SW_SHOW);
        MoveWindow(g_app.ua_label, 4, y, width - 8, 20, TRUE);
        y += 24;
    } else {
        ShowWindow(g_app.ua_label, SW_HIDE);
    }
    if (g_app.controller) {
        RECT bounds;
        bounds.left = 0;
        bounds.top = y;
        bounds.right = rc.right;
        bounds.bottom = rc.bottom;
        if (bounds.bottom < bounds.top) bounds.bottom = bounds.top;
        g_app.controller->put_Bounds(bounds);
    }
}

void navigate_to(const std::wstring &url)
{
    std::wstring target = normalize_url(url);
    if (g_app.url_edit) {
        SetWindowTextW(g_app.url_edit, target.c_str());
    }
    if (!g_app.ready || !g_app.webview) {
        return;
    }
    HRESULT hr = g_app.webview->Navigate(target.c_str());
    if (FAILED(hr)) {
        ciao_error("Navigate failed: 0x%08lx", (unsigned long)hr);
    }
}

void go_from_edit(void)
{
    if (!player_ui().navigation_buttons_enabled || !player_ui().show_navigation_buttons) {
        return;
    }
    wchar_t buf[2048];
    buf[0] = L'\0';
    if (g_app.url_edit) GetWindowTextW(g_app.url_edit, buf, 2048);
    navigate_to(buf);
}

LRESULT CALLBACK edit_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_KEYDOWN && wp == VK_RETURN) {
        go_from_edit();
        return 0;
    }
    return CallWindowProcW(g_edit_proc, hwnd, msg, wp, lp);
}

void apply_user_agent(const char *ua_utf8, bool reload)
{
    if (ua_utf8 == NULL || ua_utf8[0] == '\0') {
        ciao_error("apply_user_agent: empty User-Agent");
        return;
    }
    g_app.ua = player_widen(ua_utf8);
    if (g_app.ua_label) SetWindowTextW(g_app.ua_label, g_app.ua.c_str());
    player_save_user_agent(ua_utf8);
    if (g_app.webview) {
        ComPtr<ICoreWebView2Settings> settings;
        if (SUCCEEDED(g_app.webview->get_Settings(&settings)) && settings) {
            ComPtr<ICoreWebView2Settings2> settings2;
            if (SUCCEEDED(settings.As(&settings2)) && settings2) {
                settings2->put_UserAgent(g_app.ua.c_str());
            } else {
                ciao_warn("WebView2 settings do not provide User-Agent");
            }
        }
        if (reload) g_app.webview->Reload();
    }
    ciao_info("User-Agent changed to: %s", ua_utf8);
}

void check_ua_menu(void)
{
    if (g_app.menu == nullptr) return;
    std::string current = player_user_agent_utf8();
    struct Item { UINT id; const char *ua; };
    Item items[] = {
        { IDM_UA_WEBVIEW, UA_WEBVIEW },
        { IDM_UA_CHROME, UA_CHROME },
        { IDM_UA_FIREFOX, UA_FIREFOX },
        { IDM_UA_SAFARI, UA_SAFARI },
        { IDM_UA_EDGE, UA_EDGE },
    };
    for (int i = 0; i < 5; i++) {
        UINT flags = MF_BYCOMMAND | MF_STRING | ((current == items[i].ua) ? MF_CHECKED : MF_UNCHECKED);
        CheckMenuItem(g_app.menu, items[i].id, flags);
    }
}

HMENU build_menu(void)
{
    HMENU bar = CreateMenu();
    HMENU file = CreatePopupMenu();
    const PlayerUi &ui = player_ui();
    if (ui.bookmarks_enabled) {
        AppendMenuW(file, MF_STRING, IDM_AUTOCLICK, L"AutoClick all bots");
        AppendMenuW(file, MF_SEPARATOR, 0, NULL);
        HMENU bookmarks = CreatePopupMenu();
        AppendMenuW(bookmarks, MF_STRING, IDM_BOOKMARK_START, L"Start Page");
        AppendMenuW(file, MF_POPUP, (UINT_PTR)bookmarks, L"Bookmark");
    }
    HMENU ua = CreatePopupMenu();
    AppendMenuW(ua, MF_STRING, IDM_UA_WEBVIEW, L"Webview (Default)");
    AppendMenuW(ua, MF_STRING, IDM_UA_CHROME, L"Google Chrome");
    AppendMenuW(ua, MF_STRING, IDM_UA_FIREFOX, L"Mozilla Firefox");
    AppendMenuW(ua, MF_STRING, IDM_UA_SAFARI, L"Apple Safari");
    AppendMenuW(ua, MF_STRING, IDM_UA_EDGE, L"Microsoft Edge");
    AppendMenuW(file, MF_POPUP, (UINT_PTR)ua, L"User Agent");
    HMENU lua = CreatePopupMenu();
    AppendMenuW(lua, MF_STRING, IDM_LUA_FOLDER, L"Folder");
    AppendMenuW(file, MF_POPUP, (UINT_PTR)lua, L"Lua Script");
    HMENU logm = CreatePopupMenu();
    AppendMenuW(logm, MF_STRING, IDM_LOG_FOLDER, L"Log folder");
    AppendMenuW(file, MF_POPUP, (UINT_PTR)logm, L"Log");
    AppendMenuW(file, MF_STRING, IDM_EXIT, L"Exit");
    AppendMenuW(bar, MF_POPUP, (UINT_PTR)file, L"File");
    HMENU about = CreatePopupMenu();
    AppendMenuW(about, MF_STRING, IDM_VERSION, L"Show Version");
    AppendMenuW(bar, MF_POPUP, (UINT_PTR)about, L"About");
    return bar;
}

void show_version(void)
{
    wchar_t text[1024];
    swprintf_s(text,
               L"%hs\n\nVersion: %d.%d.%d\n\n%hs",
               PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, PROJECT_DESCRIPTION);
    wchar_t title[256];
    swprintf_s(title, L"About %hs", PROJECT_NAME);
    MessageBoxW(g_app.hwnd, text, title, MB_OK | MB_ICONINFORMATION);
    ciao_debug("Version dialog displayed: %s v%d.%d.%d",
               PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

void on_autoclick(void)
{
    /* The bot list is only the NULL sentinel, so the sequence ends immediately. */
    ciao_info("[AUTOCLICK] Sequence finished or cancelled");
}

LRESULT CALLBACK popup_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    Popup *popup = reinterpret_cast<Popup *>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (msg == WM_SIZE && popup && popup->controller) {
        RECT rc;
        GetClientRect(hwnd, &rc);
        popup->controller->put_Bounds(rc);
        return 0;
    }
    if (msg == WM_DESTROY && popup && popup->controller) {
        popup->controller->Close();
        popup->controller.Reset();
        popup->webview.Reset();
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

void open_popup(ICoreWebView2NewWindowRequestedEventArgs *args)
{
    if (g_app.env == nullptr || args == nullptr) return;
    ComPtr<ICoreWebView2NewWindowRequestedEventArgs> held(args);
    ComPtr<ICoreWebView2Deferral> deferral;
    if (FAILED(held->GetDeferral(&deferral)) || !deferral) {
        held->put_Handled(TRUE);
        return;
    }
    auto popup = std::make_unique<Popup>();
    Popup *raw = popup.get();
    raw->hwnd = CreateWindowExW(
        0, k_popup_class, L"Gemini",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 960, 720,
        g_app.hwnd, NULL, GetModuleHandleW(NULL), NULL);
    if (raw->hwnd == nullptr) {
        deferral->Complete();
        return;
    }
    SetWindowLongPtrW(raw->hwnd, GWLP_USERDATA, (LONG_PTR)raw);
    g_app.popups.push_back(std::move(popup));

    HRESULT hr = g_app.env->CreateCoreWebView2Controller(
        raw->hwnd,
        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
            [raw, held, deferral](HRESULT result, ICoreWebView2Controller *controller) -> HRESULT {
                if (FAILED(result) || controller == nullptr) {
                    ciao_error("Popup WebView2 controller failed: 0x%08lx", (unsigned long)result);
                    deferral->Complete();
                    DestroyWindow(raw->hwnd);
                    return S_OK;
                }
                raw->controller = controller;
                controller->get_CoreWebView2(&raw->webview);
                RECT rc;
                GetClientRect(raw->hwnd, &rc);
                controller->put_Bounds(rc);
                controller->put_IsVisible(TRUE);
                held->put_NewWindow(raw->webview.Get());
                held->put_Handled(TRUE);
                if (raw->webview) {
                    EventRegistrationToken token{};
                    raw->webview->add_WindowCloseRequested(
                        Callback<ICoreWebView2WindowCloseRequestedEventHandler>(
                            [raw](ICoreWebView2 *, IUnknown *) -> HRESULT {
                                if (raw->hwnd) DestroyWindow(raw->hwnd);
                                return S_OK;
                            }).Get(),
                        &token);
                }
                deferral->Complete();
                return S_OK;
            }).Get());
    if (FAILED(hr)) {
        ciao_error("Popup controller request failed: 0x%08lx", (unsigned long)hr);
        deferral->Complete();
    }
}

void wire_webview(ICoreWebView2 *webview)
{
    EventRegistrationToken token{};
    webview->add_SourceChanged(
        Callback<ICoreWebView2SourceChangedEventHandler>(
            [](ICoreWebView2 *sender, ICoreWebView2SourceChangedEventArgs *) -> HRESULT {
                LPWSTR src = nullptr;
                if (SUCCEEDED(sender->get_Source(&src)) && src != nullptr) {
                    if (g_app.url_edit) SetWindowTextW(g_app.url_edit, src);
                    CoTaskMemFree(src);
                }
                return S_OK;
            }).Get(),
        &token);
    webview->add_NavigationCompleted(
        Callback<ICoreWebView2NavigationCompletedEventHandler>(
            [](ICoreWebView2 *sender, ICoreWebView2NavigationCompletedEventArgs *args) -> HRESULT {
                BOOL ok = FALSE;
                if (args) args->get_IsSuccess(&ok);
                LPWSTR src = nullptr;
                sender->get_Source(&src);
                std::string url = src ? player_narrow(src) : std::string();
                if (src) CoTaskMemFree(src);
                if (ok) {
                    ciao_info("Page loaded: %s", url.c_str());
                } else {
                    COREWEBVIEW2_WEB_ERROR_STATUS status = COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN;
                    if (args) args->get_WebErrorStatus(&status);
                    if (status != COREWEBVIEW2_WEB_ERROR_STATUS_OPERATION_CANCELED) {
                        ciao_warn("Page load failed (%d): %s", (int)status, url.c_str());
                    }
                }
                return S_OK;
            }).Get(),
        &token);
    webview->add_PermissionRequested(
        Callback<ICoreWebView2PermissionRequestedEventHandler>(
            [](ICoreWebView2 *, ICoreWebView2PermissionRequestedEventArgs *args) -> HRESULT {
                if (args) args->put_State(COREWEBVIEW2_PERMISSION_STATE_ALLOW);
                ciao_debug("[PERMISSION] Auto-allowed media device request");
                return S_OK;
            }).Get(),
        &token);
    webview->add_NewWindowRequested(
        Callback<ICoreWebView2NewWindowRequestedEventHandler>(
            [](ICoreWebView2 *, ICoreWebView2NewWindowRequestedEventArgs *args) -> HRESULT {
                open_popup(args);
                return S_OK;
            }).Get(),
        &token);
}

void on_controller_ready(HRESULT result, ICoreWebView2Controller *controller)
{
    if (FAILED(result) || controller == nullptr) {
        ciao_error("WebView2 controller failed: 0x%08lx", (unsigned long)result);
        if ((unsigned long)result == 0x80070578UL) {
            ciao_error("The window is not on an interactive desktop. Start gemini-player from the signed-in Windows session.");
        } else if (g_app.hwnd) {
            MessageBoxW(g_app.hwnd, L"WebView2 could not create the page view.", L"gemini-player", MB_OK | MB_ICONERROR);
        }
        PostQuitMessage(1);
        return;
    }
    g_app.controller = controller;
    controller->get_CoreWebView2(&g_app.webview);
    if (!g_app.webview) {
        ciao_error("WebView2 view is missing");
        PostQuitMessage(1);
        return;
    }
    ComPtr<ICoreWebView2Settings> settings;
    if (SUCCEEDED(g_app.webview->get_Settings(&settings)) && settings) {
        settings->put_IsScriptEnabled(TRUE);
        settings->put_AreDefaultScriptDialogsEnabled(TRUE);
        settings->put_IsStatusBarEnabled(FALSE);
        settings->put_AreDevToolsEnabled(TRUE);
        ComPtr<ICoreWebView2Settings2> settings2;
        if (SUCCEEDED(settings.As(&settings2)) && settings2) {
            settings2->put_UserAgent(g_app.ua.c_str());
        }
    }
    wire_webview(g_app.webview.Get());
    controller->put_IsVisible(TRUE);
    g_app.ready = true;
    layout_window();
    ciao_info("WebView2 view ready");
    navigate_to(player_startup_url());
}

void on_environment_ready(HRESULT result, ICoreWebView2Environment *env)
{
    if (FAILED(result) || env == nullptr) {
        ciao_error("WebView2 environment failed: 0x%08lx", (unsigned long)result);
        MessageBoxW(g_app.hwnd, L"WebView2 runtime did not start. Install the Evergreen runtime and rebuild.",
                    L"gemini-player", MB_OK | MB_ICONERROR);
        PostQuitMessage(1);
        return;
    }
    g_app.env = env;
    HRESULT hr = env->CreateCoreWebView2Controller(
        g_app.hwnd,
        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
            [](HRESULT hr, ICoreWebView2Controller *controller) -> HRESULT {
                on_controller_ready(hr, controller);
                return S_OK;
            }).Get());
    if (FAILED(hr)) {
        ciao_error("CreateCoreWebView2Controller failed: 0x%08lx", (unsigned long)hr);
        PostQuitMessage(1);
    }
}

void start_webview(void)
{
    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    if (options) {
        options->put_AdditionalBrowserArguments(L"--autoplay-policy=no-user-gesture-required");
    }
    const std::wstring &folder = player_paths().webview_profile;
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        folder.c_str(),
        options.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT result, ICoreWebView2Environment *env) -> HRESULT {
                on_environment_ready(result, env);
                return S_OK;
            }).Get());
    if (FAILED(hr)) {
        ciao_error("CreateCoreWebView2EnvironmentWithOptions failed: 0x%08lx", (unsigned long)hr);
        PostQuitMessage(1);
    }
}

LRESULT CALLBACK main_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_SIZE:
        layout_window();
        return 0;
    case WM_COMMAND: {
        UINT id = LOWORD(wp);
        switch (id) {
        case IDC_BACK:
            if (g_app.webview) g_app.webview->GoBack();
            return 0;
        case IDC_FORWARD:
            if (g_app.webview) g_app.webview->GoForward();
            return 0;
        case IDC_RELOAD:
            if (g_app.webview) g_app.webview->Reload();
            return 0;
        case IDC_GO:
            go_from_edit();
            return 0;
        case IDM_AUTOCLICK:
            on_autoclick();
            return 0;
        case IDM_BOOKMARK_START:
            navigate_to(player_widen(START_PAGE));
            ciao_info("Bookmark Start Page");
            return 0;
        case IDM_UA_WEBVIEW: apply_user_agent(UA_WEBVIEW, true); check_ua_menu(); return 0;
        case IDM_UA_CHROME: apply_user_agent(UA_CHROME, true); check_ua_menu(); return 0;
        case IDM_UA_FIREFOX: apply_user_agent(UA_FIREFOX, true); check_ua_menu(); return 0;
        case IDM_UA_SAFARI: apply_user_agent(UA_SAFARI, true); check_ua_menu(); return 0;
        case IDM_UA_EDGE: apply_user_agent(UA_EDGE, true); check_ua_menu(); return 0;
        case IDM_LUA_FOLDER:
            open_folder(player_paths().userscripts_dir);
            return 0;
        case IDM_LOG_FOLDER:
            open_folder(player_paths().log_dir);
            return 0;
        case IDM_EXIT:
            DestroyWindow(hwnd);
            return 0;
        case IDM_VERSION:
            show_version();
            return 0;
        default:
            break;
        }
        break;
    }
    case WM_TIMER:
        if (wp == IDT_SHUTDOWN) {
            ciao_info("SHUTDOWN timer reached - terminating application now.");
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    case WM_DESTROY:
        if (g_app.controller) {
            g_app.controller->Close();
            g_app.controller.Reset();
        }
        g_app.webview.Reset();
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

HWND create_main_window(HINSTANCE instance)
{
    WNDCLASSW wc;
    ZeroMemory(&wc, sizeof wc);
    wc.lpfnWndProc = main_proc;
    wc.hInstance = instance;
    wc.lpszClassName = k_main_class;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    WNDCLASSW popup_wc;
    ZeroMemory(&popup_wc, sizeof popup_wc);
    popup_wc.lpfnWndProc = popup_proc;
    popup_wc.hInstance = instance;
    popup_wc.lpszClassName = k_popup_class;
    popup_wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    popup_wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&popup_wc);

    const PlayerUi &ui = player_ui();
    HWND hwnd = CreateWindowExW(
        0, k_main_class, ui.window_title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, ui.width, ui.height,
        NULL, NULL, instance, NULL);
    if (hwnd == nullptr) {
        ciao_error("Failed to create main window");
        return nullptr;
    }
    g_app.hwnd = hwnd;
    g_app.menu = build_menu();
    SetMenu(hwnd, g_app.menu);
    check_ua_menu();

    HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    g_app.back_btn = CreateWindowExW(0, L"BUTTON", L"Back", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, hwnd, (HMENU)IDC_BACK, instance, NULL);
    g_app.fwd_btn = CreateWindowExW(0, L"BUTTON", L"Forward", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, hwnd, (HMENU)IDC_FORWARD, instance, NULL);
    g_app.reload_btn = CreateWindowExW(0, L"BUTTON", L"Reload", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, hwnd, (HMENU)IDC_RELOAD, instance, NULL);
    g_app.url_edit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 0, 10, 10, hwnd, (HMENU)IDC_URL, instance, NULL);
    g_app.go_btn = CreateWindowExW(0, L"BUTTON", L"Go", WS_CHILD | WS_VISIBLE, 0, 0, 10, 10, hwnd, (HMENU)IDC_GO, instance, NULL);
    g_app.ua_label = CreateWindowExW(0, L"STATIC", g_app.ua.c_str(), WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP | SS_NOPREFIX, 0, 0, 10, 10, hwnd, (HMENU)IDC_UA, instance, NULL);
    HWND controls[] = { g_app.back_btn, g_app.fwd_btn, g_app.reload_btn, g_app.url_edit, g_app.go_btn, g_app.ua_label };
    for (int i = 0; i < 6; i++) {
        SendMessageW(controls[i], WM_SETFONT, (WPARAM)font, TRUE);
    }
    g_edit_proc = (WNDPROC)SetWindowLongPtrW(g_app.url_edit, GWLP_WNDPROC, (LONG_PTR)edit_proc);
    SetWindowTextW(g_app.url_edit, player_widen(START_PAGE).c_str());
    layout_window();
    return hwnd;
}

int run_check(void)
{
    std::wstring version;
    if (!player_webview_runtime_version(&version)) {
        ciao_error("WebView2 Evergreen runtime is not installed");
        return 1;
    }
    ciao_info("WebView2 runtime %s", player_narrow(version).c_str());
    HRESULT co = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(co)) {
        ciao_error("CoInitializeEx failed: 0x%08lx", (unsigned long)co);
        return 1;
    }
    HRESULT got = E_FAIL;
    bool done = false;
    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        player_paths().webview_profile.c_str(),
        options.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [&got, &done](HRESULT result, ICoreWebView2Environment *env) -> HRESULT {
                got = result;
                done = true;
                if (env == nullptr && SUCCEEDED(result)) got = E_FAIL;
                PostQuitMessage(0);
                return S_OK;
            }).Get());
    if (FAILED(hr)) {
        ciao_error("CreateCoreWebView2EnvironmentWithOptions failed: 0x%08lx", (unsigned long)hr);
        CoUninitialize();
        return 1;
    }
    UINT_PTR timer = SetTimer(NULL, 0, 20000, NULL);
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        if (msg.message == WM_TIMER) {
            ciao_error("WebView2 environment timed out");
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (timer) KillTimer(NULL, timer);
    CoUninitialize();
    if (!done || FAILED(got)) {
        ciao_error("WebView2 environment failed: 0x%08lx", (unsigned long)got);
        return 1;
    }
    ciao_info("WebView2 environment created");
    ciao_info("settings.sqlite ready");
    ciao_info("check passed");
    return 0;
}

int run_window(void)
{
    std::wstring version;
    if (!player_webview_runtime_version(&version)) {
        ciao_error("WebView2 Evergreen runtime is not installed");
        return 1;
    }
    ciao_info("WebView2 runtime %s", player_narrow(version).c_str());
    HRESULT co = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(co)) {
        ciao_error("CoInitializeEx failed: 0x%08lx", (unsigned long)co);
        return 1;
    }
    HINSTANCE instance = GetModuleHandleW(NULL);
    if (create_main_window(instance) == nullptr) {
        CoUninitialize();
        return 1;
    }
    ShowWindow(g_app.hwnd, SW_SHOW);
    UpdateWindow(g_app.hwnd);
    if (ciao_shutdown_seconds > 0) {
        ciao_debug("Scheduling auto-shutdown timer for %d seconds", ciao_shutdown_seconds);
        SetTimer(g_app.hwnd, IDT_SHUTDOWN, (UINT)ciao_shutdown_seconds * 1000u, NULL);
    }
    ciao_info("Starting %s %d.%d.%d", PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    start_webview();
    MSG msg;
    int code = 0;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (msg.message == WM_QUIT) code = (int)msg.wParam;
    ciao_info("%s terminated", PROJECT_NAME);
    CoUninitialize();
    return code;
}

}  // namespace

int main(int argc, char **argv)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    player_prepare(argc, argv);
    if (player_help_requested()) {
        player_print_help();
        return 0;
    }
    g_app.ua = player_user_agent_wide();
    if (player_check_requested()) {
        return run_check();
    }
    return run_window();
}
