#pragma once
#include <filesystem>
#include <string>

namespace Bolt {
	enum class SpecialFolder {
		User,              
		Desktop,            
		Documents,         
		Downloads,          
		Music,              
		Pictures,          
		Videos,           
		AppDataRoaming,   
		LocalAppData,      
		ProgramData
	};

	class Path {
	public:
		static std::string GetSpecialFolderPath(SpecialFolder folder);

		template <typename... Args>
		static std::string Combine(Args&&... args) {
			std::filesystem::path combined;
			((combined /= std::filesystem::path(std::forward<Args>(args))), ...);
			return combined.make_preferred().string();
		}

		static std::string Current() {
			return std::filesystem::current_path().string();
		}

		static std::string ExecutableDir();
	};
}