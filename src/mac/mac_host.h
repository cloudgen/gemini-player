// mac_host.h
// Profile paths, config.conf, and settings.sqlite for the macOS player.

#ifndef MAC_HOST_H
#define MAC_HOST_H

#include <stdbool.h>
#include <string>

struct PlayerPaths {
    std::string home;
    std::string app_root;
    std::string config_path;
    std::string storage_root;
    std::string settings_path;
    std::string webkit_dir;
    std::string log_dir;
    std::string userscripts_dir;
    std::string cookies_dir;
};

struct PlayerUi {
    std::string window_title;
    int width;
    int height;
    bool show_navigation_buttons;
    bool show_user_agent_label;
    bool navigation_buttons_enabled;
    bool bookmarks_enabled;
    bool cookies_enabled;
    bool scripts_enabled;
};

void player_prepare(int argc, char **argv);

bool player_help_requested(void);
bool player_check_requested(void);

const PlayerPaths &player_paths(void);
const PlayerUi &player_ui(void);

std::string player_user_agent_utf8(void);
void player_save_user_agent(const char *ua_utf8);
std::string player_startup_url(void);
void player_print_help(void);

#endif
