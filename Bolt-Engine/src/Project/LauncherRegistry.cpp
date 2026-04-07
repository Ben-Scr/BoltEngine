#include "pch.hpp"
#include "Project/LauncherRegistry.hpp"
#include "Project/BoltProject.hpp"
#include "Serialization/Path.hpp"
#include "Serialization/Directory.hpp"
#include "Serialization/File.hpp"
#include "Core/Log.hpp"

#include <algorithm>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <filesystem>

namespace Bolt {

	static std::string NowISO8601() {
		auto t = std::time(nullptr);
		std::tm tm{};
		localtime_s(&tm, &t);
		std::stringstream ss;
		ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
		return ss.str();
	}

	// Minimal JSON helpers — no external library needed for these simple structures
	static std::string EscapeJSON(const std::string& s) {
		std::string out;
		for (char c : s) {
			if (c == '\\') out += "\\\\";
			else if (c == '"') out += "\\\"";
			else out += c;
		}
		return out;
	}

	static std::string ExtractValue(const std::string& json, const std::string& key) {
		std::string search = "\"" + key + "\"";
		auto pos = json.find(search);
		if (pos == std::string::npos) return "";

		pos = json.find('"', pos + search.size() + 1);
		if (pos == std::string::npos) return "";

		auto end = json.find('"', pos + 1);
		if (end == std::string::npos) return "";

		return json.substr(pos + 1, end - pos - 1);
	}

	std::string LauncherRegistry::GetRegistryPath() {
		return Path::Combine(
			Path::GetSpecialFolderPath(SpecialFolder::LocalAppData),
			"Bolt", "launcher.json");
	}

	void LauncherRegistry::Load() {
		m_Projects.clear();
		std::string path = GetRegistryPath();

		if (!File::Exists(path)) return;

		std::ifstream in(path);
		std::string json((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
		in.close();

		// Parse simple JSON array of objects
		size_t pos = 0;
		while ((pos = json.find('{', pos)) != std::string::npos) {
			auto end = json.find('}', pos);
			if (end == std::string::npos) break;

			std::string obj = json.substr(pos, end - pos + 1);

			LauncherProjectEntry entry;
			entry.name = ExtractValue(obj, "name");
			entry.path = ExtractValue(obj, "path");
			entry.lastOpened = ExtractValue(obj, "lastOpened");

			if (!entry.name.empty() && !entry.path.empty())
				m_Projects.push_back(entry);

			pos = end + 1;
		}

		std::sort(m_Projects.begin(), m_Projects.end(),
			[](const auto& a, const auto& b) { return a.lastOpened > b.lastOpened; });
	}

	void LauncherRegistry::Save() {
		std::string path = GetRegistryPath();

		// Ensure parent directory exists
		auto parent = std::filesystem::path(path).parent_path();
		if (!std::filesystem::exists(parent))
			std::filesystem::create_directories(parent);

		std::stringstream ss;
		ss << "[\n";
		for (size_t i = 0; i < m_Projects.size(); i++) {
			auto& p = m_Projects[i];
			ss << "  { \"name\": \"" << EscapeJSON(p.name)
			   << "\", \"path\": \"" << EscapeJSON(p.path)
			   << "\", \"lastOpened\": \"" << p.lastOpened << "\" }";
			if (i + 1 < m_Projects.size()) ss << ",";
			ss << "\n";
		}
		ss << "]\n";

		File::WriteAllText(path, ss.str());
	}

	void LauncherRegistry::AddProject(const std::string& name, const std::string& path) {
		for (auto& p : m_Projects)
			if (p.path == path) { p.lastOpened = NowISO8601(); return; }

		LauncherProjectEntry entry;
		entry.name = name;
		entry.path = path;
		entry.lastOpened = NowISO8601();
		m_Projects.insert(m_Projects.begin(), entry);
	}

	void LauncherRegistry::RemoveProject(const std::string& path) {
		m_Projects.erase(
			std::remove_if(m_Projects.begin(), m_Projects.end(),
				[&](const auto& e) { return e.path == path; }),
			m_Projects.end());
	}

	void LauncherRegistry::UpdateLastOpened(const std::string& path) {
		for (auto& p : m_Projects) {
			if (p.path == path) {
				p.lastOpened = NowISO8601();
				break;
			}
		}
		std::sort(m_Projects.begin(), m_Projects.end(),
			[](const auto& a, const auto& b) { return a.lastOpened > b.lastOpened; });
	}

	void LauncherRegistry::ValidateAll() {
		m_Projects.erase(
			std::remove_if(m_Projects.begin(), m_Projects.end(),
				[](const auto& e) { return !BoltProject::Validate(e.path); }),
			m_Projects.end());
	}

} // namespace Bolt
