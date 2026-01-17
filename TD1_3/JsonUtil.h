#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <Novice.h>

#include "json.hpp"

using json = nlohmann::json;

/// <summary>
/// JSON読み込み/保存の汎用ユーティリティ
/// </summary>
class JsonUtil {
public:
	/// <summary>
	/// JSONファイルを読み込む
	/// </summary>
	/// <param name="filepath">読み込むファイルのパス</param>
	/// <param name="outJson">読み込んだJSONデータの格納先</param>
	/// <returns>成功した場合true、失敗した場合false</returns>
	static bool LoadFromFile(const std::string& filepath, json& outJson) {
#ifdef _DEBUG
		try {
			std::ifstream file(filepath);
			if (!file.is_open()) {
				Novice::ConsolePrintf("JsonUtil: File not found: %s\n", filepath.c_str());
				return false;
			}

			outJson = json::parse(file);
			Novice::ConsolePrintf("JsonUtil: Loaded: %s\n", filepath.c_str());
			return true;
		}
		catch (const json::parse_error& e) {

			Novice::ConsolePrintf("JsonUtil: Parse error: %s\n", e.what());

			return false;
		}
		catch (const std::exception& e) {


			Novice::ConsolePrintf("JsonUtil: Error: %s\n", e.what());

			return false;
		}
#else
		try {
			std::ifstream file(filepath);
			if (!file.is_open()) {
				return false;
			}
			outJson = json::parse(file);
			return true;
		}
		catch (...) {
			return false;
		}
#endif
	}

	/// <summary>
	/// JSONファイルに保存
	/// </summary>
	/// <param name="filepath">保存先のファイルパス</param>
	/// <param name="j">保存するJSONデータ</param>
	/// <returns>成功した場合true、失敗した場合false</returns>
	static bool SaveToFile(const std::string& filepath, const json& j) {
		try {
			std::ofstream file(filepath);
			if (!file.is_open()) {
#ifdef _DEBUG
				Novice::ConsolePrintf("JsonUtil: Failed to open for writing: %s\n", filepath.c_str());
#endif
				return false;
			}

			file << j.dump(1);
#ifdef _DEBUG
			Novice::ConsolePrintf("JsonUtil: Saved: %s\n", filepath.c_str());
#endif
			return true;
		}
		catch (const std::exception& e) {
			e;
#ifdef _DEBUG
			Novice::ConsolePrintf("JsonUtil: Write error: %s\n", e.what());
#endif
			return false;
		}
	}

	/// <summary>
	/// マップデータ専用：コンパクトフォーマットで保存
	/// </summary>
	static bool SaveMapCompact(const std::string& filepath, const json& j) {
		try {
			std::ofstream file(filepath);
			if (!file.is_open()) {
#ifdef _DEBUG
				Novice::ConsolePrintf("JsonUtil: Failed to open for writing: %s\n", filepath.c_str());
#endif
				return false;
			}

			file << "{\n";

			// メタデータ
			file << " \"width\": " << j["width"] << ",\n";
			file << " \"height\": " << j["height"] << ",\n";
			file << " \"tileSize\": " << j["tileSize"] << ",\n";

			file << " \"layers\": {\n";

			// タイルレイヤー
			const std::vector<std::string> layerNames = { "background", "decoration", "block" };
			bool firstLayer = true;

			for (const auto& layerName : layerNames) {
				if (!j["layers"].contains(layerName)) continue;

				if (!firstLayer) file << ",\n";
				firstLayer = false;

				file << "  \"" << layerName << "\": [\n";

				const auto& layer = j["layers"][layerName];
				for (size_t y = 0; y < layer.size(); ++y) {
					file << "   [";

					const auto& row = layer[y];
					for (size_t x = 0; x < row.size(); ++x) {
						file << row[x];
						if (x < row.size() - 1) file << ",";
					}

					file << "]";
					if (y < layer.size() - 1) file << ",";
					file << "\n";
				}

				file << "  ]";
			}

			file << "\n },\n";

			// オブジェクトスポーン情報
			file << " \"objects\": [\n";
			if (j.contains("objects")) {
				const auto& objects = j["objects"];
				for (size_t i = 0; i < objects.size(); ++i) {
					const auto& obj = objects[i];
					file << "  {\"type\":" << obj["type"]
						<< ",\"position\":{\"x\":" << obj["position"]["x"]
						<< ",\"y\":" << obj["position"]["y"] << "}";

					if (obj.contains("tag")) {
						file << ",\"tag\":\"" << obj["tag"].get<std::string>() << "\"";
					}
					if (obj.contains("data") && !obj["data"].empty()) {
						file << ",\"data\":" << obj["data"].dump();
					}

					file << "}";
					if (i < objects.size() - 1) file << ",";
					file << "\n";
				}
			}
			file << " ]\n";

			file << "}\n";

#ifdef _DEBUG
			Novice::ConsolePrintf("JsonUtil: Saved compact map: %s\n", filepath.c_str());
#endif
			return true;
		}
		catch (const std::exception& e) {
#ifdef _DEBUG
			Novice::ConsolePrintf("JsonUtil: Write error: %s\n", e.what());
#endif
			return false;
		}
	}

	/// <summary>
	/// JSONから値を安全に取得（デフォルト値付き）
	/// </summary>
	/// <typeparam name="T">取得する値の型</typeparam>
	/// <param name="j">JSONオブジェクト</param>
	/// <param name="key">キー名</param>
	/// <param name="defaultValue">デフォルト値</param>
	/// <returns>取得した値、またはデフォルト値</returns>
	template<typename T>
	static T GetValue(const json& j, const std::string& key, const T& defaultValue) {
		try {
			if (j.contains(key) && !j[key].is_null()) {
				return j[key].get<T>();
			}
		}
		catch (const std::exception& e) {
			e;
#ifdef _DEBUG
			Novice::ConsolePrintf("JsonUtil: Failed to get value for key '%s': %s\n", key.c_str(), e.what());
#endif
		}
		return defaultValue;
	}

	/// <summary>
	/// JSONに値を設定
	/// </summary>
	/// <typeparam name="T">設定する値の型</typeparam>
	/// <param name="j">JSONオブジェクト</param>
	/// <param name="key">キー名</param>
	/// <param name="value">設定する値</param>
	template<typename T>
	static void SetValue(json& j, const std::string& key, const T& value) {
		j[key] = value;
	}
};