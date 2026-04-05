#include <pch.hpp>
#include "Path.hpp"

#ifdef BT_PLATFORM_WINDOWS
#include <windows.h>
#include <shlobj.h>
#else
#include <cstdlib>
#include <unistd.h>
#include <limits.h>
#endif

namespace Bolt {
    std::string Path::GetSpecialFolderPath(SpecialFolder folder)
    {
#ifdef BT_PLATFORM_WINDOWS
        PWSTR path = nullptr;
        HRESULT hr = E_FAIL;

        switch (folder) {
        case SpecialFolder::User: hr = SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &path); break;
        case SpecialFolder::Desktop: hr = SHGetKnownFolderPath(FOLDERID_Desktop, 0, nullptr, &path); break;
        case SpecialFolder::Documents: hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &path); break;
        case SpecialFolder::Downloads: hr = SHGetKnownFolderPath(FOLDERID_Downloads, 0, nullptr, &path); break;
        case SpecialFolder::Music: hr = SHGetKnownFolderPath(FOLDERID_Music, 0, nullptr, &path); break;
        case SpecialFolder::Pictures: hr = SHGetKnownFolderPath(FOLDERID_Pictures, 0, nullptr, &path); break;
        case SpecialFolder::Videos: hr = SHGetKnownFolderPath(FOLDERID_Videos, 0, nullptr, &path); break;
        case SpecialFolder::AppDataRoaming: hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path); break;
        case SpecialFolder::LocalAppData: hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path); break;
        case SpecialFolder::ProgramData: hr = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, nullptr, &path); break;
        default: BT_THROW(BoltErrorCode::InvalidArgument, "Unknown SpecialFolder");
        }

        BT_ASSERT(!FAILED(hr), BoltErrorCode::Undefined, "Failed to get folder path");

        const int size = WideCharToMultiByte(CP_UTF8, 0, path, -1, nullptr, 0, nullptr, nullptr);
        std::string result(size - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, path, -1, &result[0], size, nullptr, nullptr);

        CoTaskMemFree(path);
        return result;
#else
        const char* home = std::getenv("HOME");
        if (!home)
            BT_THROW(BoltErrorCode::Undefined, "HOME environment variable is not set");

        switch (folder) {
        case SpecialFolder::User: return home;
        case SpecialFolder::Desktop: return Combine(home, "Desktop");
        case SpecialFolder::Documents: return Combine(home, "Documents");
        case SpecialFolder::Downloads: return Combine(home, "Downloads");
        case SpecialFolder::Music: return Combine(home, "Music");
        case SpecialFolder::Pictures: return Combine(home, "Pictures");
        case SpecialFolder::Videos: return Combine(home, "Videos");
        case SpecialFolder::AppDataRoaming:
        case SpecialFolder::LocalAppData:
            return std::getenv("XDG_CONFIG_HOME") ? std::getenv("XDG_CONFIG_HOME") : Combine(home, ".config");
        case SpecialFolder::ProgramData:
            return "/usr/share";
        default:
            BT_THROW(BoltErrorCode::InvalidArgument, "Unknown SpecialFolder");
        }
#endif
    }

    // F-11: Return the directory that contains the running executable.
    std::string Path::ExecutableDir()
    {
#ifdef BT_PLATFORM_WINDOWS
        wchar_t buf[MAX_PATH]{};
        GetModuleFileNameW(nullptr, buf, MAX_PATH);
        return std::filesystem::path(buf).parent_path().string();
#else
        char buf[PATH_MAX]{};
        ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
        if (len <= 0)
            BT_THROW(BoltErrorCode::Undefined, "Failed to resolve executable path");
        buf[len] = '\0';
        return std::filesystem::path(buf).parent_path().string();
#endif
    }
}