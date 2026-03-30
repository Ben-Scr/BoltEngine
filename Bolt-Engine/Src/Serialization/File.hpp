#pragma once
#include "Debugging/Exceptions.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace Bolt {
	class File {
	public:
		File() = delete;

		static bool Exists(const std::string& path);

		static void WriteAllText(const std::string& path, const std::string& text);

		template<size_t Size>
		static void WriteAllLines(const std::string& path, const std::array<std::string, Size> lines) {
			std::ofstream file(path);

			BT_ASSERT(file.is_open(), BoltErrorCode::Undefined(), "File couldn't be opened");

			for (const std::string& line : lines) {
				file.write(line.c_str(), line.size());
				file.write("\n", 1);
			}

			file.close();
		}

		template<size_t Size>
		static void WriteAllBytes(const std::string& path, const std::array<std::uint8_t, Size> bytes) {
			std::ofstream file(path, std::ios::binary);

			BT_ASSERT(file.is_open(), BoltErrorCode::Undefined(), "File couldn't be opened");

			file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
			file.close();
		}

		template<typename T>
		static void WriteObject(const std::string& path, const T& obj) {
			std::ofstream file(path, std::ios::out | std::ios::binary);

			BT_ASSERT(file.is_open(), BoltErrorCode::Undefined(), "File couldn't be opened");

			file.write(reinterpret_cast<const char*>(&obj), sizeof(T));
		}

		template<typename T>
		static T ReadObject(const std::string& path) {
			std::ifstream file(path, std::ios::binary);

			BT_ASSERT(file.is_open(), BoltErrorCode::Undefined(), "File couldn't be opened");

			T obj{};
			file.read(reinterpret_cast<char*>(&obj), sizeof(T));
			return obj;
		}

		static std::string ReadAllText(const std::string& path);
		static std::vector<std::uint8_t> ReadAllBytes(const std::string& path);
	};
}
