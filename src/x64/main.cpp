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

    void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
    {
        switch (a_msg->type) {
        case SKSE::MessagingInterface::kPostLoad:
            // все SKSE-плагины уже загружены, экспорт TrueHUD доступен.
            // v0.1.2: сторож (StartWatcher) почти наверняка уже поставил перехват,
            // здесь остаётся дешёвая страховка на случай неудачи сторожа.
            if (!Arbiter::Hook::Install()) {
                logger::warn("[SBA] hook install failed at kPostLoad, will retry at kPostPostLoad");
            }
            break;
        case SKSE::MessagingInterface::kPostPostLoad:
            Arbiter::Hook::Install();  // идемпотентно; последняя попытка при неудаче
            break;
        default:
            break;
        }
    }
}

extern "C" [[maybe_unused]] __declspec(dllexport) bool SKSEPlugin_Load(SKSE::LoadInterface* a_intfc)
{
    SetupLog();
    SKSE::Init(a_intfc);

    logger::info("Special Bar Arbiter v0.1.2 (Bouncer) loading...");
    Arbiter::Config::Load();

    // v0.1.2 «Bouncer»: ставим перехват как можно раньше — сразу, как только
    // TrueHUD.dll появится в памяти. Пока сторож не стоит, моды, получающие
    // kPostLoad раньше нас, успевают взять настоящий API в обход шима.
    Arbiter::Hook::StartWatcher();

    SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);

    logger::info("Special Bar Arbiter loaded.");
    return true;
}
