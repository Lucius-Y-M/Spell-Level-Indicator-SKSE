#pragma once

#include "../include/Process.h"
#include "Process.cpp" // for some reason, if I don't do this, CMake Build fails. If you know why, please let me know.




#define DLLEXPORT __declspec(dllexport)

void InitializeLog([[maybe_unused]] spdlog::level::level_enum a_level = spdlog::level::info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		util::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= std::format("{}.log"sv, Plugin::NAME);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	const auto level = a_level;

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s:%#] %v");
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	logger::info("Loaded plugin {} {}", Plugin::NAME, Plugin::VERSION.string());
	SKSE::Init(a_skse);



	constexpr auto visitors = Overloaded (
		[] (NONE) {
			CONSOLE_PRINT(">> Spell Level Indicator SLAAD initialized successfully.");
		},

		[] (Str errMsg) {
			CONSOLE_PRINT("!! Failed to initialize Spell Level Indicator SLAAD, no changes have been made. Err Msg:\n=== %s ===\n Read Log file for more info.",
				errMsg
			);
		}
	);

	SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message * msg) {

		if (msg->type == SKSE::MessagingInterface::kDataLoaded) {
			
			Result<NONE, Str> result = Patch::ProcessLoadOrder();

			std::visit(
				visitors,
				result
			);
		
		}


	});


	return true;
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName(Plugin::NAME.data());
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary(true);
	v.HasNoStructUse();
	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}