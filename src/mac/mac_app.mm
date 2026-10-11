// mac_app.mm
// Win32-equivalent window for macOS: Cocoa chrome and a WKWebView.

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

#include "player_project.h"
#include "mac_host.h"
#include "mac_log.h"

static const char *k_ua_names[] = {
    "Webview (Default)",
    "Google Chrome",
    "Mozilla Firefox",
    "Apple Safari",
    "Microsoft Edge"
};
static const char *k_ua_values[] = {
    UA_WEBVIEW,
    UA_CHROME,
    UA_FIREFOX,
    UA_SAFARI,
    UA_EDGE
};

static WKWebView *g_web = nil;
static NSTextField *g_url = nil;
static NSTextField *g_ua_label = nil;
static NSMutableArray *g_popups = nil;

static NSString *ns_utf8(const std::string &text)
{
    return [NSString stringWithUTF8String:text.c_str()];
}

static void apply_user_agent(const char *ua)
{
    if (ua == NULL || g_web == nil) {
        return;
    }
    g_web.customUserAgent = ns_utf8(ua);
    if (g_ua_label != nil) {
        g_ua_label.stringValue = ns_utf8(ua);
    }
    player_save_user_agent(ua);
    if (g_web.URL != nil) {
        [g_web reload];
    }
}

static void load_url_string(NSString *raw)
{
    if (raw == nil) {
        return;
    }
    NSString *trimmed = [raw stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
    if (trimmed.length == 0) {
        return;
    }
    if ([trimmed rangeOfString:@"://"].location == NSNotFound) {
        trimmed = [@"https://" stringByAppendingString:trimmed];
    }
    NSURL *url = [NSURL URLWithString:trimmed];
    if (url == nil) {
        ciao_warn("URL could not be parsed");
        return;
    }
    [g_web loadRequest:[NSURLRequest requestWithURL:url]];
}

static void open_folder(const std::string &path)
{
    NSURL *url = [NSURL fileURLWithPath:ns_utf8(path) isDirectory:YES];
    [[NSWorkspace sharedWorkspace] openURL:url];
}

@interface PlayerController : NSObject <NSApplicationDelegate, NSWindowDelegate, WKNavigationDelegate, WKUIDelegate>
@property (nonatomic, strong) NSWindow *window;
@property (nonatomic, strong) NSButton *backButton;
@property (nonatomic, strong) NSButton *forwardButton;
@property (nonatomic, strong) NSButton *reloadButton;
@property (nonatomic, strong) NSButton *goButton;
- (void)layoutChrome;
- (void)goClicked:(id)sender;
- (void)backClicked:(id)sender;
- (void)forwardClicked:(id)sender;
- (void)reloadClicked:(id)sender;
- (void)startPageClicked:(id)sender;
- (void)userAgentClicked:(id)sender;
- (void)openScripts:(id)sender;
- (void)openLogs:(id)sender;
- (void)autoClickClicked:(id)sender;
- (void)aboutClicked:(id)sender;
@end

@implementation PlayerController

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
    (void)note;
    const PlayerUi &ui = player_ui();
    NSRect frame = NSMakeRect(80, 80, ui.width, ui.height);
    self.window = [[NSWindow alloc] initWithContentRect:frame
                                               styleMask:(NSWindowStyleMaskTitled |
                                                          NSWindowStyleMaskClosable |
                                                          NSWindowStyleMaskMiniaturizable |
                                                          NSWindowStyleMaskResizable)
                                                 backing:NSBackingStoreBuffered
                                                   defer:NO];
    self.window.title = ns_utf8(ui.window_title);
    self.window.delegate = self;
    NSView *content = self.window.contentView;

    self.backButton = [NSButton buttonWithTitle:@"Back" target:self action:@selector(backClicked:)];
    self.forwardButton = [NSButton buttonWithTitle:@"Forward" target:self action:@selector(forwardClicked:)];
    self.reloadButton = [NSButton buttonWithTitle:@"Reload" target:self action:@selector(reloadClicked:)];
    self.goButton = [NSButton buttonWithTitle:@"Go" target:self action:@selector(goClicked:)];
    g_url = [[NSTextField alloc] initWithFrame:NSZeroRect];
    g_url.placeholderString = @"https://";
    g_url.stringValue = ns_utf8(player_startup_url());
    g_ua_label = [[NSTextField alloc] initWithFrame:NSZeroRect];
    g_ua_label.editable = NO;
    g_ua_label.bezeled = NO;
    g_ua_label.drawsBackground = NO;
    g_ua_label.stringValue = ns_utf8(player_user_agent_utf8());
    g_ua_label.lineBreakMode = NSLineBreakByTruncatingTail;

    WKWebViewConfiguration *cfg = [[WKWebViewConfiguration alloc] init];
    cfg.websiteDataStore = [WKWebsiteDataStore defaultDataStore];
    cfg.mediaTypesRequiringUserActionForPlayback = WKAudiovisualMediaTypeNone;
    g_web = [[WKWebView alloc] initWithFrame:NSZeroRect configuration:cfg];
    g_web.navigationDelegate = self;
    g_web.UIDelegate = self;
    g_web.customUserAgent = ns_utf8(player_user_agent_utf8());

    if (ui.show_navigation_buttons) {
        [content addSubview:self.backButton];
        [content addSubview:self.forwardButton];
        [content addSubview:self.reloadButton];
    }
    [content addSubview:g_url];
    [content addSubview:self.goButton];
    if (ui.show_user_agent_label) {
        [content addSubview:g_ua_label];
    }
    [content addSubview:g_web];

    BOOL nav = ui.navigation_buttons_enabled ? YES : NO;
    self.backButton.enabled = nav;
    self.forwardButton.enabled = nav;
    self.reloadButton.enabled = nav;

    [self layoutChrome];
    [self.window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
    load_url_string(ns_utf8(player_startup_url()));
    ciao_info("Starting %s %d.%d.%d", PROJECT_ID, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    if (ciao_shutdown_seconds > 0) {
        int seconds = ciao_shutdown_seconds;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)seconds * NSEC_PER_SEC),
                       dispatch_get_main_queue(), ^{
            ciao_info("SHUTDOWN timer reached - terminating application now.");
            [NSApp terminate:nil];
        });
    }
}

- (void)layoutChrome
{
    NSRect bounds = self.window.contentView.bounds;
    CGFloat y = bounds.size.height - 32;
    CGFloat x = 8;
    const PlayerUi &ui = player_ui();
    if (ui.show_navigation_buttons) {
        self.backButton.frame = NSMakeRect(x, y, 64, 24);
        x += 68;
        self.forwardButton.frame = NSMakeRect(x, y, 76, 24);
        x += 80;
        self.reloadButton.frame = NSMakeRect(x, y, 72, 24);
        x += 76;
    }
    CGFloat ua_w = ui.show_user_agent_label ? 220 : 0;
    CGFloat go_w = 44;
    CGFloat url_w = bounds.size.width - x - go_w - ua_w - 24;
    if (url_w < 80) url_w = 80;
    g_url.frame = NSMakeRect(x, y, url_w, 24);
    x += url_w + 6;
    self.goButton.frame = NSMakeRect(x, y, go_w, 24);
    x += go_w + 6;
    if (ui.show_user_agent_label) {
        g_ua_label.frame = NSMakeRect(x, y, ua_w, 24);
    }
    g_web.frame = NSMakeRect(0, 0, bounds.size.width, y - 8);
}

- (void)windowDidResize:(NSNotification *)notification
{
    (void)notification;
    [self layoutChrome];
}

- (void)goClicked:(id)sender
{
    (void)sender;
    load_url_string(g_url.stringValue);
}

- (void)backClicked:(id)sender
{
    (void)sender;
    if ([g_web canGoBack]) [g_web goBack];
}

- (void)forwardClicked:(id)sender
{
    (void)sender;
    if ([g_web canGoForward]) [g_web goForward];
}

- (void)reloadClicked:(id)sender
{
    (void)sender;
    [g_web reload];
}

- (void)startPageClicked:(id)sender
{
    (void)sender;
    load_url_string(ns_utf8(START_PAGE));
}

- (void)userAgentClicked:(id)sender
{
    NSMenuItem *item = (NSMenuItem *)sender;
    NSInteger idx = item.tag;
    if (idx < 0 || idx > 4) {
        return;
    }
    apply_user_agent(k_ua_values[idx]);
    ciao_info("User-Agent set to %s", k_ua_names[idx]);
}

- (void)openScripts:(id)sender
{
    (void)sender;
    open_folder(player_paths().userscripts_dir);
    ciao_info("Opened userscripts folder. Lua execution stays on the Ubuntu program.");
}

- (void)openLogs:(id)sender
{
    (void)sender;
    open_folder(player_paths().log_dir);
}

- (void)autoClickClicked:(id)sender
{
    (void)sender;
    ciao_info("AutoClick list is only the empty sentinel. No Follow click is scheduled.");
}

- (void)aboutClicked:(id)sender
{
    (void)sender;
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = ns_utf8(PROJECT_NAME);
    alert.informativeText = [NSString stringWithFormat:@"%d.%d.%d\n%s",
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, PROJECT_DESCRIPTION];
    [alert addButtonWithTitle:@"OK"];
    [alert runModal];
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
    (void)navigation;
    if (webView != g_web || webView.URL == nil) {
        return;
    }
    NSString *abs = webView.URL.absoluteString;
    if (abs != nil) {
        g_url.stringValue = abs;
    }
    ciao_info("WebKit view ready");
}

- (WKWebView *)webView:(WKWebView *)webView
createWebViewWithConfiguration:(WKWebViewConfiguration *)configuration
   forNavigationAction:(WKNavigationAction *)navigationAction
        windowFeatures:(WKWindowFeatures *)windowFeatures
{
    (void)webView;
    (void)navigationAction;
    (void)windowFeatures;
    if (g_popups == nil) {
        g_popups = [[NSMutableArray alloc] init];
    }
    NSRect frame = NSMakeRect(140, 140, 800, 600);
    NSWindow *popup = [[NSWindow alloc] initWithContentRect:frame
                                                   styleMask:(NSWindowStyleMaskTitled |
                                                              NSWindowStyleMaskClosable |
                                                              NSWindowStyleMaskResizable)
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
    popup.title = @"Gemini";
    WKWebView *view = [[WKWebView alloc] initWithFrame:popup.contentView.bounds configuration:configuration];
    view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    view.customUserAgent = g_web.customUserAgent;
    [popup.contentView addSubview:view];
    [popup makeKeyAndOrderFront:nil];
    [g_popups addObject:popup];
    [g_popups addObject:view];
    return view;
}

- (NSMenu *)buildMenu
{
    const PlayerUi &ui = player_ui();
    NSMenu *menubar = [[NSMenu alloc] init];

    NSMenuItem *appItem = [[NSMenuItem alloc] init];
    NSMenu *appMenu = [[NSMenu alloc] initWithTitle:ns_utf8(PROJECT_NAME)];
    NSMenuItem *aboutItem = [appMenu addItemWithTitle:[NSString stringWithFormat:@"About %@", ns_utf8(PROJECT_NAME)]
                                                action:@selector(aboutClicked:)
                                         keyEquivalent:@""];
    aboutItem.target = self;
    [appMenu addItem:[NSMenuItem separatorItem]];
    NSMenuItem *quitItem = [appMenu addItemWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
    quitItem.target = NSApp;
    appItem.submenu = appMenu;
    [menubar addItem:appItem];

    NSMenuItem *fileItem = [[NSMenuItem alloc] init];
    NSMenu *fileMenu = [[NSMenu alloc] initWithTitle:@"File"];
    if (ui.bookmarks_enabled) {
        NSMenuItem *autoItem = [fileMenu addItemWithTitle:@"AutoClick all bots" action:@selector(autoClickClicked:) keyEquivalent:@""];
        autoItem.target = self;
        NSMenuItem *bookmark = [[NSMenuItem alloc] initWithTitle:@"Bookmark" action:NULL keyEquivalent:@""];
        NSMenu *bookmarkMenu = [[NSMenu alloc] initWithTitle:@"Bookmark"];
        NSMenuItem *start = [[NSMenuItem alloc] initWithTitle:@"Start Page"
                                                       action:@selector(startPageClicked:)
                                                keyEquivalent:@""];
        start.target = self;
        [bookmarkMenu addItem:start];
        bookmark.submenu = bookmarkMenu;
        [fileMenu addItem:bookmark];
    }
    NSMenuItem *ua = [[NSMenuItem alloc] initWithTitle:@"User Agent" action:NULL keyEquivalent:@""];
    NSMenu *uaMenu = [[NSMenu alloc] initWithTitle:@"User Agent"];
    for (NSInteger i = 0; i < 5; i++) {
        NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:ns_utf8(k_ua_names[i])
                                                      action:@selector(userAgentClicked:)
                                               keyEquivalent:@""];
        item.target = self;
        item.tag = i;
        [uaMenu addItem:item];
    }
    ua.submenu = uaMenu;
    [fileMenu addItem:ua];
    NSMenuItem *scripts = [[NSMenuItem alloc] initWithTitle:@"Lua Script" action:@selector(openScripts:) keyEquivalent:@""];
    scripts.target = self;
    [fileMenu addItem:scripts];
    NSMenuItem *logs = [[NSMenuItem alloc] initWithTitle:@"Log" action:@selector(openLogs:) keyEquivalent:@""];
    logs.target = self;
    [fileMenu addItem:logs];
    NSMenuItem *exitItem = [fileMenu addItemWithTitle:@"Exit" action:@selector(terminate:) keyEquivalent:@""];
    exitItem.target = NSApp;
    fileItem.submenu = fileMenu;
    [menubar addItem:fileItem];
    return menubar;
}

@end

int main(int argc, char **argv)
{
    @autoreleasepool {
        player_prepare(argc, argv);
        if (player_help_requested()) {
            player_print_help();
            return 0;
        }
        if (player_check_requested()) {
            ciao_info("macOS WebKit profile ready at %s", player_paths().app_root.c_str());
            printf("check passed\n");
            fflush(stdout);
            return 0;
        }
        NSApplication *app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        PlayerController *controller = [[PlayerController alloc] init];
        app.delegate = controller;
        app.mainMenu = [controller buildMenu];
        [app run];
        return 0;
    }
}
