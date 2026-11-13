#pragma once

namespace Arbiter
{
    // Прозрачный посредник между клиентскими плагинами и настоящим TrueHUD API.
    // Реализует IVTrueHUD4 (совместим по vtable и с V1..V3, т.к. наследует тот же класс).
    // Функции "запроса контроля" пропускают только плагин-владельца из конфига.
    // Функции спец-полосок без handle (FlashActorValue, цвета) БЕЗОПАСНО глохнут,
    // пока полоски никем не заняты — это и есть фикс краша std::bad_function_call.
    class SpecialBarShim : public TRUEHUD_API::IVTrueHUD4
    {
    public:
        static void* Attach(void* a_realInterface, const void* a_callerReturnAddress,
                            TRUEHUD_API::InterfaceVersion a_version);

        static SpecialBarShim* GetSingleton();

        // IVTrueHUD1
        [[nodiscard]] unsigned long GetTrueHUDThreadId() const noexcept override;
        [[nodiscard]] APIResult     RequestTargetControl(SKSE::PluginHandle a_myPluginHandle) noexcept override;
        [[nodiscard]] APIResult     RequestSpecialResourceBarsControl(SKSE::PluginHandle a_myPluginHandle) noexcept override;
        APIResult                   SetTarget(SKSE::PluginHandle a_myPluginHandle, RE::ActorHandle a_actorHandle) noexcept override;
        APIResult                   SetSoftTarget(SKSE::PluginHandle a_myPluginHandle, RE::ActorHandle a_actorHandle) noexcept override;
        void                        AddActorInfoBar(RE::ActorHandle a_actorHandle) noexcept override;
        void                        RemoveActorInfoBar(RE::ActorHandle a_actorHandle, TRUEHUD_API::WidgetRemovalMode a_removalMode) noexcept override;
        void                        AddBoss(RE::ActorHandle a_actorHandle) noexcept override;
        void                        RemoveBoss(RE::ActorHandle a_actorHandle, TRUEHUD_API::WidgetRemovalMode a_removalMode) noexcept override;
        void                        FlashActorValue(RE::ActorHandle a_actorHandle, RE::ActorValue a_actorValue, bool a_bLong) noexcept override;
        [[nodiscard]] APIResult     FlashActorSpecialBar(SKSE::PluginHandle a_myPluginHandle, RE::ActorHandle a_actorHandle, bool a_bLong) noexcept override;
        [[nodiscard]] APIResult     RegisterSpecialResourceFunctions(SKSE::PluginHandle a_myPluginHandle, SpecialResourceCallback&& a_getCurrentSpecialResource, SpecialResourceCallback&& a_getMaxSpecialResource, bool a_bSpecialMode, bool a_bDisplaySpecialForPlayer = true) noexcept override;
        void                        LoadCustomWidgets(SKSE::PluginHandle a_myPluginHandle, std::string_view a_filePath, APIResultCallback&& a_successCallback) noexcept override;
        void                        RegisterNewWidgetType(SKSE::PluginHandle a_myPluginHandle, std::uint32_t a_widgetType) noexcept override;
        void                        AddWidget(SKSE::PluginHandle a_myPluginHandle, std::uint32_t a_widgetType, std::uint32_t a_widgetID, std::string_view a_symbolIdentifier, std::shared_ptr<TRUEHUD_API::WidgetBase> a_widget) noexcept override;
        void                        RemoveWidget(SKSE::PluginHandle a_myPluginHandle, std::uint32_t a_widgetType, std::uint32_t a_widgetID, TRUEHUD_API::WidgetRemovalMode a_removalMode) noexcept override;
        [[nodiscard]] SKSE::PluginHandle GetTargetControlOwner() const noexcept override;
        [[nodiscard]] SKSE::PluginHandle GetPlayerWidgetBarColorsControlOwner() const noexcept override;
        [[nodiscard]] SKSE::PluginHandle GetSpecialResourceBarControlOwner() const noexcept override;
        [[nodiscard]] APIResult     ReleaseTargetControl(SKSE::PluginHandle a_myPluginHandle) noexcept override;
        [[nodiscard]] APIResult     ReleaseSpecialResourceBarControl(SKSE::PluginHandle a_myPluginHandle) noexcept override;

        // IVTrueHUD2
        void OverrideBarColor(RE::ActorHandle a_actorHandle, RE::ActorValue a_actorValue, TRUEHUD_API::BarColorType a_colorType, std::uint32_t a_color) noexcept override;
        void OverrideSpecialBarColor(RE::ActorHandle a_actorHandle, TRUEHUD_API::BarColorType a_colorType, std::uint32_t a_color) noexcept override;
        void RevertBarColor(RE::ActorHandle a_actorHandle, RE::ActorValue a_actorValue, TRUEHUD_API::BarColorType a_colorType) noexcept override;
        void RevertSpecialBarColor(RE::ActorHandle a_actorHandle, TRUEHUD_API::BarColorType a_colorType) noexcept override;

        // IVTrueHUD3
        void DrawLine(const RE::NiPoint3& a_start, const RE::NiPoint3& a_end, float a_duration = 0.f, std::uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f) noexcept override;
        void DrawPoint(const RE::NiPoint3& a_position, float a_size, float a_duration = 0.f, std::uint32_t a_color = 0xFF0000FF) noexcept override;
        void DrawArrow(const RE::NiPoint3& a_start, const RE::NiPoint3& a_end, float a_size = 10.f, float a_duration = 0.f, std::uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f) noexcept override;
        void DrawBox(const RE::NiPoint3& a_center, const RE::NiPoint3& a_extent, const RE::NiQuaternion& a_rotation, float a_duration = 0.f, std::uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f) noexcept override;
        void DrawCircle(const RE::NiPoint3& a_center, const RE::NiPoint3& a_x, const RE::NiPoint3& a_y, float a_radius, std::uint32_t a_segments, float a_duration = 0.f, std::uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f) noexcept override;
        void DrawHalfCircle(const RE::NiPoint3& a_center, const RE::NiPoint3& a_x, const RE::NiPoint3& a_y, float a_radius, std::uint32_t a_segments, float a_duration = 0.f, std::uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f) noexcept override;
        void DrawSphere(const RE::NiPoint3& a_origin, float a_radius, std::uint32_t a_segments = 16, float a_duration = 0.f, std::uint32_t a_color = 0xFF0000FF) noexcept override;
        void DrawCylinder(const RE::NiPoint3& a_start, const RE::NiPoint3& a_end, float a_radius, std::uint32_t a_segments, float a_duration = 0.f, std::uint32_t a_color = 0xFF0000FF) noexcept override;
        void DrawCone(const RE::NiPoint3& a_origin, const RE::NiPoint3& a_direction, float a_length, float a_angleWidth, float a_angleHeight, std::uint32_t a_segments, float a_duration = 0.f, std::uint32_t a_color = 0xFF0000FF) noexcept override;
        void DrawCapsule(const RE::NiPoint3& a_center, const RE::NiPoint3& a_extent, const RE::NiQuaternion& a_rotation, float a_duration = 0.f, std::uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f) noexcept override;
        [[nodiscard]] bool HasInfoBar(RE::ActorHandle a_actorHandle, bool a_bFloatingOnly = false) const noexcept override;

        // IVTrueHUD4
        void DrawCapsule(const RE::NiPoint3& a_vertexA, const RE::NiPoint3& a_vertexB, float a_radius, float a_duration = 0.f, std::uint32_t a_color = 0xFF0000FF, float a_thickness = 1.f) noexcept override;

    private:
        explicit SpecialBarShim(TRUEHUD_API::IVTrueHUD4* a_real);

        TRUEHUD_API::IVTrueHUD4* _real{ nullptr };
        bool                     _specialClaimed{ false };
        std::wstring             _specialOwnerModule;
        std::recursive_mutex     _lock;
    };
}
