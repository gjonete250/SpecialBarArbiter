#include "PCH.h"
#include "Shim.h"
#include "Config.h"

namespace Arbiter
{
    namespace
    {
        // Владелец спец-ресурсов с точки зрения Шима (модуль, чей запрос прошёл)
        std::recursive_mutex g_attachLock;
    }

    SpecialBarShim::SpecialBarShim(TRUEHUD_API::IVTrueHUD4* a_real) :
        _real(a_real)
    {}

    SpecialBarShim*& SpecialBarShim::GetSingleton()
    {
        static SpecialBarShim* s = nullptr;
        return s;
    }

    void* SpecialBarShim::Attach(void* a_realInterface, const void* a_callerReturnAddress,
                                 TRUEHUD_API::InterfaceVersion a_version)
    {
        if (!a_realInterface) return nullptr;

        auto callerModule = Config::GetCallerModuleName(a_callerReturnAddress);
        std::lock_guard lock(g_attachLock);

        if (!GetSingleton()) {
            // создаём единственный Шим поверх настоящего интерфейса
            static SpecialBarShim instance(reinterpret_cast<TRUEHUD_API::IVTrueHUD4*>(a_realInterface));
            GetSingleton() = &instance;
            logger::info("[SBA] API interface attached (shim created)");
        }

        logger::info("[SBA] RequestPluginAPI(v{}) from '{}' -> shim",
                     static_cast<int>(a_version), std::string(callerModule.begin(), callerModule.end()));
        return GetSingleton();
    }

    // ---------- IVTrueHUD1 ----------

    unsigned long SpecialBarShim::GetTrueHUDThreadId() const noexcept
    {
        return _real->GetTrueHUDThreadId();
    }

    APIResult SpecialBarShim::RequestTargetControl(SKSE::PluginHandle a_myPluginHandle) noexcept
    {
        const void* caller = _ReturnAddress();
        std::wstring callerModule;
        if (Config::IsCallerTargetControlOwner(caller, &callerModule)) {
            auto r = _real->RequestTargetControl(a_myPluginHandle);
            logger::info("[SBA] RequestTargetControl({}) from '{}' -> OK forwarding",
                         a_myPluginHandle, std::string(callerModule.begin(), callerModule.end()));
            return r;
        }
        logger::info("[SBA] RequestTargetControl DENIED to '{}' (target owner is '{}')",
                     std::string(callerModule.begin(), callerModule.end()),
                     std::string(Config::GetSingleton().targetControlOwner.begin(), Config::GetSingleton().targetControlOwner.end()));
        return APIResult::AlreadyTaken;
    }

    APIResult SpecialBarShim::RequestSpecialResourceBarsControl(SKSE::PluginHandle a_myPluginHandle) noexcept
    {
        const void* caller = _ReturnAddress();
        std::wstring callerModule;
        if (Config::IsCallerOwner(caller, &callerModule)) {
            auto r = _real->RequestSpecialResourceBarsControl(a_myPluginHandle);
            if (r == APIResult::OK) {
                std::lock_guard lock(_lock);
                _specialClaimed = true;
                _specialOwnerModule = callerModule;
            }
            logger::info("[SBA] RequestSpecialResourceBarsControl({}) from '{}' -> forwarded, result={}",
                         a_myPluginHandle, std::string(callerModule.begin(), callerModule.end()), static_cast<int>(r));
            return r;
        }
        logger::info("[SBA] RequestSpecialResourceBarsControl DENIED to '{}' (owner is '{}')",
                     std::string(callerModule.begin(), callerModule.end()),
                     std::string(Config::GetSingleton().specialBarOwner.begin(), Config::GetSingleton().specialBarOwner.end()));
        return APIResult::AlreadyTaken;
    }

    APIResult SpecialBarShim::SetTarget(SKSE::PluginHandle a_myPluginHandle, RE::ActorHandle a_actorHandle) noexcept
    {
        const void* caller = _ReturnAddress();
        if (Config::IsCallerTargetControlOwner(caller))
            return _real->SetTarget(a_myPluginHandle, a_actorHandle);
        return APIResult::NotOwner;
    }

    APIResult SpecialBarShim::SetSoftTarget(SKSE::PluginHandle a_myPluginHandle, RE::ActorHandle a_actorHandle) noexcept
    {
        const void* caller = _ReturnAddress();
        if (Config::IsCallerTargetControlOwner(caller))
            return _real->SetSoftTarget(a_myPluginHandle, a_actorHandle);
        return APIResult::NotOwner;
    }

    void SpecialBarShim::AddActorInfoBar(RE::ActorHandle a_actorHandle) noexcept
    {
        _real->AddActorInfoBar(a_actorHandle);
    }

    void SpecialBarShim::RemoveActorInfoBar(RE::ActorHandle a_actorHandle, TRUEHUD_API::WidgetRemovalMode a_removalMode) noexcept
    {
        _real->RemoveActorInfoBar(a_actorHandle, a_removalMode);
    }

    void SpecialBarShim::AddBoss(RE::ActorHandle a_actorHandle) noexcept
    {
        _real->AddBoss(a_actorHandle);
    }

    void SpecialBarShim::RemoveBoss(RE::ActorHandle a_actorHandle, TRUEHUD_API::WidgetRemovalMode a_removalMode) noexcept
    {
        _real->RemoveBoss(a_actorHandle, a_removalMode);
    }

    // ГЛАВНЫЙ ФИКС: FlashActorValue без handle — источник краша std::bad_function_call,
    // когда спец-полоски никем не заняты. Глохнем безопасно, если полоски не заняты.
    void SpecialBarShim::FlashActorValue(RE::ActorHandle a_actorHandle, RE::ActorValue a_actorValue, bool a_bLong) noexcept
    {
        std::lock_guard lock(_lock);
        if (!_specialClaimed) {
            if (Config::GetSingleton().verboseLog)
                logger::info("[SBA] FlashActorValue swallowed (special bars unclaimed) - crash prevented");
            return;
        }
        _real->FlashActorValue(a_actorHandle, a_actorValue, a_bLong);
    }

    APIResult SpecialBarShim::FlashActorSpecialBar(SKSE::PluginHandle a_myPluginHandle, RE::ActorHandle a_actorHandle, bool a_bLong) noexcept
    {
        const void* caller = _ReturnAddress();
        if (Config::IsCallerOwner(caller))
            return _real->FlashActorSpecialBar(a_myPluginHandle, a_actorHandle, a_bLong);
        return APIResult::NotOwner;
    }

    APIResult SpecialBarShim::RegisterSpecialResourceFunctions(SKSE::PluginHandle a_myPluginHandle, SpecialResourceCallback&& a_getCurrentSpecialResource, SpecialResourceCallback&& a_getMaxSpecialResource, bool a_bSpecialMode, bool a_bDisplaySpecialForPlayer) noexcept
    {
        const void* caller = _ReturnAddress();
        if (Config::IsCallerOwner(caller))
            return _real->RegisterSpecialResourceFunctions(a_myPluginHandle, std::move(a_getCurrentSpecialResource), std::move(a_getMaxSpecialResource), a_bSpecialMode, a_bDisplaySpecialForPlayer);
        return APIResult::NotOwner;
    }

    void SpecialBarShim::LoadCustomWidgets(SKSE::PluginHandle a_myPluginHandle, std::string_view a_filePath, APIResultCallback&& a_successCallback) noexcept
    {
        const void* caller = _ReturnAddress();
        if (Config::IsCallerOwner(caller))
            _real->LoadCustomWidgets(a_myPluginHandle, a_filePath, std::move(a_successCallback));
    }

    void SpecialBarShim::RegisterNewWidgetType(SKSE::PluginHandle a_myPluginHandle, std::uint32_t a_widgetType) noexcept
    {
        const void* caller = _ReturnAddress();
        if (Config::IsCallerOwner(caller))
            _real->RegisterNewWidgetType(a_myPluginHandle, a_widgetType);
    }

    void SpecialBarShim::AddWidget(SKSE::PluginHandle a_myPluginHandle, std::uint32_t a_widgetType, std::uint32_t a_widgetID, std::string_view a_symbolIdentifier, std::shared_ptr<TRUEHUD_API::WidgetBase> a_widget) noexcept
    {
        const void* caller = _ReturnAddress();
        if (Config::IsCallerOwner(caller))
            _real->AddWidget(a_myPluginHandle, a_widgetType, a_widgetID, a_symbolIdentifier, std::move(a_widget));
    }

    void SpecialBarShim::RemoveWidget(SKSE::PluginHandle a_myPluginHandle, std::uint32_t a_widgetType, std::uint32_t a_widgetID, TRUEHUD_API::WidgetRemovalMode a_removalMode) noexcept
    {
        const void* caller = _ReturnAddress();
        if (Config::IsCallerOwner(caller))
            _real->RemoveWidget(a_myPluginHandle, a_widgetType, a_widgetID, a_removalMode);
    }

    SKSE::PluginHandle SpecialBarShim::GetTargetControlOwner() const noexcept
    {
        return _real->GetTargetControlOwner();
    }

    SKSE::PluginHandle SpecialBarShim::GetPlayerWidgetBarColorsControlOwner() const noexcept
    {
        return _real->GetPlayerWidgetBarColorsControlOwner();
    }

    SKSE::PluginHandle SpecialBarShim::GetSpecialResourceBarControlOwner() const noexcept
    {
        return _real->GetSpecialResourceBarControlOwner();
    }

    APIResult SpecialBarShim::ReleaseTargetControl(SKSE::PluginHandle a_myPluginHandle) noexcept
    {
        return _real->ReleaseTargetControl(a_myPluginHandle);
    }

    APIResult SpecialBarShim::ReleaseSpecialResourceBarControl(SKSE::PluginHandle a_myPluginHandle) noexcept
    {
        auto r = _real->ReleaseSpecialResourceBarControl(a_myPluginHandle);
        if (r == APIResult::OK) {
            std::lock_guard lock(_lock);
            _specialClaimed = false;
            _specialOwnerModule.clear();
        }
        return r;
    }

    // ---------- IVTrueHUD2 ----------

    void SpecialBarShim::OverrideBarColor(RE::ActorHandle a_actorHandle, RE::ActorValue a_actorValue, TRUEHUD_API::BarColorType a_colorType, std::uint32_t a_color) noexcept
    {
        _real->OverrideBarColor(a_actorHandle, a_actorValue, a_colorType, a_color);
    }

    void SpecialBarShim::OverrideSpecialBarColor(RE::ActorHandle a_actorHandle, TRUEHUD_API::BarColorType a_colorType, std::uint32_t a_color) noexcept
    {
        std::lock_guard lock(_lock);
        if (!_specialClaimed) {
            if (Config::GetSingleton().verboseLog)
                logger::info("[SBA] OverrideSpecialBarColor swallowed (special bars unclaimed) - crash prevented");
            return;
        }
        _real->OverrideSpecialBarColor(a_actorHandle, a_colorType, a_color);
    }

    void SpecialBarShim::RevertBarColor(RE::ActorHandle a_actorHandle, RE::ActorValue a_actorValue, TRUEHUD_API::BarColorType a_colorType) noexcept
    {
        _real->RevertBarColor(a_actorHandle, a_actorValue, a_colorType);
    }

    void SpecialBarShim::RevertSpecialBarColor(RE::ActorHandle a_actorHandle, TRUEHUD_API::BarColorType a_colorType) noexcept
    {
        std::lock_guard lock(_lock);
        if (!_specialClaimed) {
            if (Config::GetSingleton().verboseLog)
                logger::info("[SBA] RevertSpecialBarColor swallowed (special bars unclaimed) - crash prevented");
            return;
        }
        _real->RevertSpecialBarColor(a_actorHandle, a_colorType);
    }

    // ---------- IVTrueHUD3 ----------

    void SpecialBarShim::DrawLine(const RE::NiPoint3& a_start, const RE::NiPoint3& a_end, float a_duration, std::uint32_t a_color, float a_thickness) noexcept
    {
        _real->DrawLine(a_start, a_end, a_duration, a_color, a_thickness);
    }

    void SpecialBarShim::DrawPoint(const RE::NiPoint3& a_position, float a_size, float a_duration, std::uint32_t a_color) noexcept
    {
        _real->DrawPoint(a_position, a_size, a_duration, a_color);
    }

    void SpecialBarShim::DrawArrow(const RE::NiPoint3& a_start, const RE::NiPoint3& a_end, float a_size, float a_duration, std::uint32_t a_color, float a_thickness) noexcept
    {
        _real->DrawArrow(a_start, a_end, a_size, a_duration, a_color, a_thickness);
    }

    void SpecialBarShim::DrawBox(const RE::NiPoint3& a_center, const RE::NiPoint3& a_extent, const RE::NiQuaternion& a_rotation, float a_duration, std::uint32_t a_color, float a_thickness) noexcept
    {
        _real->DrawBox(a_center, a_extent, a_rotation, a_duration, a_color, a_thickness);
    }

    void SpecialBarShim::DrawCircle(const RE::NiPoint3& a_center, const RE::NiPoint3& a_x, const RE::NiPoint3& a_y, float a_radius, std::uint32_t a_segments, float a_duration, std::uint32_t a_color, float a_thickness) noexcept
    {
        _real->DrawCircle(a_center, a_x, a_y, a_radius, a_segments, a_duration, a_color, a_thickness);
    }

    void SpecialBarShim::DrawHalfCircle(const RE::NiPoint3& a_center, const RE::NiPoint3& a_x, const RE::NiPoint3& a_y, float a_radius, std::uint32_t a_segments, float a_duration, std::uint32_t a_color, float a_thickness) noexcept
    {
        _real->DrawHalfCircle(a_center, a_x, a_y, a_radius, a_segments, a_duration, a_color, a_thickness);
    }

    void SpecialBarShim::DrawSphere(const RE::NiPoint3& a_origin, float a_radius, std::uint32_t a_segments, float a_duration, std::uint32_t a_color, float a_thickness) noexcept
    {
        _real->DrawSphere(a_origin, a_radius, a_segments, a_duration, a_color, a_thickness);
    }

    void SpecialBarShim::DrawCylinder(const RE::NiPoint3& a_start, const RE::NiPoint3& a_end, float a_radius, std::uint32_t a_segments, float a_duration, std::uint32_t a_color, float a_thickness) noexcept
    {
        _real->DrawCylinder(a_start, a_end, a_radius, a_segments, a_duration, a_color, a_thickness);
    }

    void SpecialBarShim::DrawCone(const RE::NiPoint3& a_origin, const RE::NiPoint3& a_direction, float a_length, float a_angleWidth, float a_angleHeight, std::uint32_t a_segments, float a_duration, std::uint32_t a_color, float a_thickness) noexcept
    {
        _real->DrawCone(a_origin, a_direction, a_length, a_angleWidth, a_angleHeight, a_segments, a_duration, a_color, a_thickness);
    }

    void SpecialBarShim::DrawCapsule(const RE::NiPoint3& a_origin, float a_halfHeight, float a_radius, const RE::NiQuaternion& a_rotation, float a_duration, std::uint32_t a_color, float a_thickness) noexcept
    {
        // вызов через базовый IVTrueHUD3: имя DrawCapsule в IVTrueHUD4 скрывает перегрузку базового класса,
        // а квалифицированный вызов был бы НЕвиртуальным и упёрся в чисто виртуальную функцию
        static_cast<TRUEHUD_API::IVTrueHUD3*>(_real)->DrawCapsule(a_origin, a_halfHeight, a_radius, a_rotation, a_duration, a_color, a_thickness);
    }

    bool SpecialBarShim::HasInfoBar(RE::ActorHandle a_actorHandle, bool a_bFloatingOnly) const noexcept
    {
        return _real->HasInfoBar(a_actorHandle, a_bFloatingOnly);
    }

    // ---------- IVTrueHUD4 ----------

    void SpecialBarShim::DrawCapsule(const RE::NiPoint3& a_vertexA, const RE::NiPoint3& a_vertexB, float a_radius, float a_duration, std::uint32_t a_color, float a_thickness) noexcept
    {
        _real->DrawCapsule(a_vertexA, a_vertexB, a_radius, a_duration, a_color, a_thickness);
    }
}
