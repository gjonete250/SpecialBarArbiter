#include "PCH.h"
#include "Config.h"
#include "Hook.h"

namespace
{
    void SetupLog()
    {
        auto path = SKSE::log::log_directory();
        if (!path) return;
        *path /= "SpecialBarArbiter.log";
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
        auto log  = std::make_shared<spdlog::logger>("Special Bar Arbiter", std::move(sink));
        spdlog::set_default_logger(std::move(log));
        spdlog::set_level(spdlog::level::info);
        spdlog::flush_on(spdlog::level::info);
    }

    void MessageHandler(SKSE::MessagingInterface::Event* a_event)
    {
        switch (a_event->type) {
        case SKSE::MessagingInterface::kPostLoad:
            // все SKSE-плагины уже загружены, экспорт TrueHUD доступен
            if (!Arbiter::Hook::Install()) {
                logger::warn("[SBA] hook install failed at kPostLoad, will retry at kPostPostLoad");
            }
            break;
        case SKSE::MessagingInterface::kPostPostLoad:
            Arbiter::Hook::Install();  // идемпотентно; вторая попытка при неудаче
            break;
        default:
            break;
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInfo* a_info)
{
    SetupLog();
    SKSE::Init(a_info);

    logger::info("Special Bar Arbiter v0.1 (Peacemaker) loading...");
    Arbiter::Config::Load();

    SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);

    logger::info("Special Bar Arbiter loaded.");
}
