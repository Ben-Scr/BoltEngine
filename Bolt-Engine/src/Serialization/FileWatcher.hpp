#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>
#include <functional>
#include <chrono>

namespace Bolt {

	/// <summary>
	/// Polls a directory for file changes at a configurable interval.
	/// Uses std::filesystem::last_write_time — no platform-specific APIs.
	///
	/// Usage:
	///   FileWatcher watcher;
	///   watcher.Watch("path/to/dir", ".cs", [](){ /* rebuild */ });
	///   // In update loop:
	///   watcher.Poll();
	/// </summary>
	class FileWatcher {
	public:
		using Callback = std::function<void()>;

		/// Start watching a directory for files matching the given extension.
		/// callback is invoked when any matching file is created, modified, or deleted.
		void Watch(const std::string& directory, const std::string& extension, Callback callback);

		/// Stop watching.
		void Stop();

		/// Check for changes. Call this periodically (e.g. once per frame).
		/// Internally throttled to pollIntervalMs.
		void Poll(float pollIntervalSeconds = 1.0f);

		bool IsWatching() const { return m_Watching; }

	private:
		void ScanFiles();

		std::string m_Directory;
		std::string m_Extension;
		Callback m_Callback;
		bool m_Watching = false;

		// Tracked files: path -> last_write_time
		std::unordered_map<std::string, std::filesystem::file_time_type> m_FileTimestamps;

		// Throttle polling
		std::chrono::steady_clock::time_point m_LastPollTime;
	};

} // namespace Bolt
