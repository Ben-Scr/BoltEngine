#include "pch.hpp"
#include "Serialization/FileWatcher.hpp"
#include "Core/Log.hpp"

namespace Bolt {

	void FileWatcher::Watch(const std::string& directory, const std::string& extension, Callback callback)
	{
		m_Directory = directory;
		m_Extension = extension;
		m_Callback = std::move(callback);
		m_Watching = true;
		m_LastPollTime = std::chrono::steady_clock::now();

		// Initial scan to populate timestamps
		ScanFiles();

		BT_CORE_INFO_TAG("FileWatcher", "Watching '{}' for *{} changes", directory, extension);
	}

	void FileWatcher::Stop()
	{
		m_Watching = false;
		m_FileTimestamps.clear();
		m_Callback = nullptr;
	}

	void FileWatcher::Poll(float pollIntervalSeconds)
	{
		if (!m_Watching || !m_Callback) return;

		auto now = std::chrono::steady_clock::now();
		float elapsed = std::chrono::duration<float>(now - m_LastPollTime).count();
		if (elapsed < pollIntervalSeconds) return;
		m_LastPollTime = now;

		bool changed = false;

		// Check for modified or new files
		try
		{
			for (auto& dirEntry : std::filesystem::recursive_directory_iterator(m_Directory))
			{
				if (!dirEntry.is_regular_file()) continue;

				std::string ext = dirEntry.path().extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
				if (ext != m_Extension) continue;

				std::string path = dirEntry.path().string();
				auto writeTime = dirEntry.last_write_time();

				auto it = m_FileTimestamps.find(path);
				if (it == m_FileTimestamps.end())
				{
					// New file
					m_FileTimestamps[path] = writeTime;
					changed = true;
				}
				else if (it->second != writeTime)
				{
					// Modified file
					it->second = writeTime;
					changed = true;
				}
			}

			// Check for deleted files
			for (auto it = m_FileTimestamps.begin(); it != m_FileTimestamps.end(); )
			{
				if (!std::filesystem::exists(it->first))
				{
					it = m_FileTimestamps.erase(it);
					changed = true;
				}
				else
				{
					++it;
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			BT_CORE_WARN_TAG("FileWatcher", "Filesystem error: {}", e.what());
		}

		if (changed)
		{
			BT_CORE_INFO_TAG("FileWatcher", "Changes detected in {}", m_Directory);
			m_Callback();
		}
	}

	void FileWatcher::ScanFiles()
	{
		m_FileTimestamps.clear();

		try
		{
			for (auto& dirEntry : std::filesystem::recursive_directory_iterator(m_Directory))
			{
				if (!dirEntry.is_regular_file()) continue;

				std::string ext = dirEntry.path().extension().string();
				std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
				if (ext != m_Extension) continue;

				m_FileTimestamps[dirEntry.path().string()] = dirEntry.last_write_time();
			}
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			BT_CORE_WARN_TAG("FileWatcher", "Initial scan error: {}", e.what());
		}
	}

} // namespace Bolt
