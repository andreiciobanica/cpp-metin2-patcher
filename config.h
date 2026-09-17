#ifndef CONFIG_H
#define CONFIG_H

// server-specific settings, change these before building for your server
namespace PatcherConfig
{
    // root of the patch server, with the trailing slash; the lists and the
    // client files are requested from here and saved under the same relative path
    inline constexpr const char* ServerUrl = "https://patch.example.com/";

    // window and taskbar title
    inline constexpr const char* WindowTitle = "Your Server - Autopatcher";

    // started by the Start button, the patcher closes right after
    inline constexpr const char* GameExecutable = "metin2client.exe";

    // started by the Settings button
    inline constexpr const char* ConfigExecutable = "config.exe";
}

#endif // CONFIG_H
