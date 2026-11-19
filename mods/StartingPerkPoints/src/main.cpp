// StartingPerkPoints — grants a configurable number of perk points once per
// character: at the start of a new game, or the first time an existing save is
// loaded with the mod installed. Whether a save has already received its
// points is stored in the SKSE co-save, so no ESP is needed.

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>

using namespace std::literals;

namespace
{
	constexpr std::uint32_t kSerializationID = 'SPKP';
	constexpr std::uint32_t kGrantRecord = 'GRNT';
	constexpr std::uint32_t kGrantRecordVersion = 1;

	constexpr std::int32_t kDefaultPerkPoints = 3;
	constexpr std::int32_t kMaxPerkCount = 127;  // the game stores unspent perk points in a signed byte

	struct State
	{
		bool         granted = false;
		std::int32_t amountGranted = 0;
	} g_state;

	void InitLogging()
	{
		auto dir = SKSE::log::log_directory();
		if (!dir) {
			return;
		}
		auto path = *dir / "StartingPerkPoints.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path.string(), true);
		auto logger = std::make_shared<spdlog::logger>("global", std::move(sink));
		logger->set_level(spdlog::level::info);
		logger->flush_on(spdlog::level::info);
		logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
		spdlog::set_default_logger(std::move(logger));
	}

	std::string Trim(std::string s)
	{
		auto notSpace = [](unsigned char c) { return !std::isspace(c); };
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
		s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
		return s;
	}

	// Reads iPerkPoints from Data/SKSE/Plugins/StartingPerkPoints.ini. The file is
	// re-read on every grant, so edits take effect without restarting the game.
	std::int32_t ReadConfiguredPerkPoints()
	{
		const auto path = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "StartingPerkPoints.ini";
		std::ifstream file(path);
		if (!file) {
			spdlog::warn("{} not found; using default of {} perk points", path.string(), kDefaultPerkPoints);
			return kDefaultPerkPoints;
		}

		std::string line;
		while (std::getline(file, line)) {
			line = Trim(line.substr(0, line.find_first_of(";#")));
			const auto eq = line.find('=');
			if (eq == std::string::npos) {
				continue;
			}
			auto key = Trim(line.substr(0, eq));
			std::ranges::transform(key, key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			if (key != "iperkpoints") {
				continue;
			}
			try {
				return std::clamp(std::stoi(Trim(line.substr(eq + 1))), 0, kMaxPerkCount);
			} catch (const std::exception&) {
				spdlog::error("iPerkPoints is not a number; using default of {}", kDefaultPerkPoints);
				return kDefaultPerkPoints;
			}
		}

		spdlog::warn("iPerkPoints missing from {}; using default of {}", path.string(), kDefaultPerkPoints);
		return kDefaultPerkPoints;
	}

	void GrantIfNeeded(std::string_view reason)
	{
		if (g_state.granted) {
			spdlog::info("{}: this character already received {} perk point(s); nothing to do", reason, g_state.amountGranted);
			return;
		}

		auto* player = RE::PlayerCharacter::GetSingleton();
		if (!player) {
			spdlog::error("{}: player not available; will try again on next load", reason);
			return;
		}

		const auto amount = ReadConfiguredPerkPoints();
		auto&      perkCount = player->GetGameStatsData().perkCount;
		const auto before = static_cast<std::int32_t>(perkCount);
		const auto after = std::min(before + amount, kMaxPerkCount);
		perkCount = static_cast<std::int8_t>(after);

		g_state.granted = true;
		g_state.amountGranted = after - before;

		spdlog::info("{}: granted {} perk point(s) ({} -> {})", reason, g_state.amountGranted, before, after);
		if (g_state.amountGranted > 0) {
			const auto text = std::format("Starting Perk Points: +{} perk point{}", g_state.amountGranted, g_state.amountGranted == 1 ? "" : "s");
			RE::DebugNotification(text.c_str());
		}
	}

	void OnSave(SKSE::SerializationInterface* a_intfc)
	{
		if (!g_state.granted) {
			return;
		}
		if (!a_intfc->OpenRecord(kGrantRecord, kGrantRecordVersion) ||
			!a_intfc->WriteRecordData(g_state.amountGranted)) {
			spdlog::error("failed to write grant record to co-save");
		}
	}

	void OnLoad(SKSE::SerializationInterface* a_intfc)
	{
		std::uint32_t type, version, length;
		while (a_intfc->GetNextRecordInfo(type, version, length)) {
			if (type == kGrantRecord && version == kGrantRecordVersion) {
				std::int32_t amount = 0;
				if (a_intfc->ReadRecordData(amount)) {
					g_state.granted = true;
					g_state.amountGranted = amount;
				}
			}
		}
	}

	void OnRevert(SKSE::SerializationInterface*)
	{
		g_state = {};
	}

	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type) {
		case SKSE::MessagingInterface::kNewGame:
			g_state = {};
			// Defer to the main loop so the new character is fully initialised.
			SKSE::GetTaskInterface()->AddTask([] { GrantIfNeeded("new game"sv); });
			break;
		case SKSE::MessagingInterface::kPostLoadGame:
			if (a_msg->data) {  // data is the load's success flag
				GrantIfNeeded("save loaded"sv);
			}
			break;
		default:
			break;
		}
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	InitLogging();
	spdlog::info("StartingPerkPoints loaded");

	auto* serialization = SKSE::GetSerializationInterface();
	serialization->SetUniqueID(kSerializationID);
	serialization->SetSaveCallback(OnSave);
	serialization->SetLoadCallback(OnLoad);
	serialization->SetRevertCallback(OnRevert);

	if (!SKSE::GetMessagingInterface()->RegisterListener(OnMessage)) {
		spdlog::critical("could not register SKSE message listener");
		return false;
	}
	return true;
}
