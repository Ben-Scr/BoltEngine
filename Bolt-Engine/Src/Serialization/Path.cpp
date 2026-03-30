#include <pch.hpp>
#include "Path.hpp"

#include <windows.h>
#include <shlobj.h>
#include <stdexcept>

namespace Bolt {
	std::string Path::GetSpecialFolderPath(SpecialFolder folder){
			PWSTR path = nullptr;
			HRESULT hr;

			switch (folder) {
			case SpecialFolder::User:
				hr = SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &path);
				break;
			case SpecialFolder::Desktop:
				hr = SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &path);
				break;
			case SpecialFolder::Documents:
				hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path);
				break;
			case SpecialFolder::Music:
				hr = SHGetKnownFolderPath(FOLDERID_Music, 0, NULL, &path);
				break;
			case SpecialFolder::Pictures:
				hr = SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &path);
				break;
			case SpecialFolder::Videos:
				hr = SHGetKnownFolderPath(FOLDERID_Videos, 0, NULL, &path);
				break;
			case SpecialFolder::AppDataRoaming:
				hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);
				break;
			case SpecialFolder::LocalAppData:
				hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path);
				break;
			case SpecialFolder::ProgramData:
					hr = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &path);
					break;
				break;
			default:
				BT_THROW(BoltErrorCode::InvalidArgument, "Unknown SpecialFolder");
			}

			BT_ASSERT(FAILED(hr), BoltErrorCode::Undefined, "Failed to get folder path");

			int size = WideCharToMultiByte(CP_UTF8, 0, path, -1, nullptr, 0, nullptr, nullptr);
			std::string result(size - 1, 0);
			WideCharToMultiByte(CP_UTF8, 0, path, -1, &result[0], size, nullptr, nullptr);

			CoTaskMemFree(path);
			return result;
	}
}