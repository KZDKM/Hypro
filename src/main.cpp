#include <hyprland/src/desktop/Workspace.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/managers/LayoutManager.hpp>
#include <hyprland/src/managers/AnimationManager.hpp>
#include <hyprland/src/config/ConfigValue.hpp>
#include <hyprland/src/render/pass/RectPassElement.hpp>
#include <hyprland/src/config/ConfigManager.hpp>

inline HANDLE pHandle;

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

Hyprutils::Memory::CSharedPointer<HOOK_CALLBACK_FN> tickHook;
Hyprutils::Memory::CSharedPointer<HOOK_CALLBACK_FN> renderHook;
Hyprutils::Memory::CSharedPointer<HOOK_CALLBACK_FN> configHook;

std::string monitor = "eDP-1";
int top = 0;
int bottom = 0;
int left = 0;
int right = 0;

PHLANIMVAR<float> maskAlpha;

void onTick() {
    for (auto& w : g_pCompositor->m_windows) {
        if (w->isFullscreen() && w->m_workspace->m_fullscreenMode == FSMODE_FULLSCREEN) {
            const auto m = w->m_monitor;
            if (m != g_pCompositor->getMonitorFromString(monitor)) continue;
            const Vector2D pos = Vector2D(m->m_position.x + (left / m->m_scale), m->m_position.y + (top / m->m_scale));
            const Vector2D size = Vector2D(m->m_size.x - (left / m->m_scale) - (right / m->m_scale), m->m_size.y - (top / m->m_scale) - (bottom / m->m_scale));
            if (w->m_realPosition->goal() != pos)
                *w->m_realPosition = pos;
            if (w->m_realSize->goal() != size)
                *w->m_realSize = size;
        }
    }

}

void renderRect(CBox box, CHyprColor color) {
    CRectPassElement::SRectData rectdata;
    rectdata.color = color;
    rectdata.box = box;
    g_pHyprRenderer->m_renderPass.add(makeShared<CRectPassElement>(rectdata));
}

void onRender(std::any args) {
    const auto renderStage = std::any_cast<eRenderStage>(args);

    if (renderStage == eRenderStage::RENDER_PRE_WINDOWS) {
        const auto m = g_pCompositor->getMonitorFromName(monitor);
        if (g_pHyprOpenGL->m_renderData.pMonitor == m) {
            if (m->m_activeWorkspace) {
                if (m->m_activeWorkspace->m_hasFullscreenWindow && m->m_activeWorkspace->m_fullscreenMode == FSMODE_FULLSCREEN && g_pInputManager->m_activeSwipe.pWorkspaceBegin == nullptr) {
                    if (maskAlpha->goal() != 1.f)
                    *maskAlpha = 1.f;
                }
                else if (maskAlpha->goal() != 0.f) {
                    *maskAlpha = 0.f;
                }
            }
            CBox monBox = CBox(m->m_position, m->m_transformedSize);
            if (maskAlpha->value() > 0.f && maskAlpha->isBeingAnimated())
                g_pHyprRenderer->damageMonitor(m);
            renderRect(monBox, CHyprColor(0, 0, 0, maskAlpha->value()));
        }
    }
}

void reloadConfig() {
    monitor = std::any_cast<Hyprlang::STRING>(HyprlandAPI::getConfigValue(pHandle, "plugin:hypro:monitor")->getValue());
    top = std::any_cast<Hyprlang::INT>(HyprlandAPI::getConfigValue(pHandle, "plugin:hypro:top")->getValue());
    bottom = std::any_cast<Hyprlang::INT>(HyprlandAPI::getConfigValue(pHandle, "plugin:hypro:bottom")->getValue());
    left = std::any_cast<Hyprlang::INT>(HyprlandAPI::getConfigValue(pHandle, "plugin:hypro:left")->getValue());
    right = std::any_cast<Hyprlang::INT>(HyprlandAPI::getConfigValue(pHandle, "plugin:hypro:right")->getValue());
}


APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE inHandle) {
    pHandle = inHandle;

    g_pAnimationManager->createAnimation(0.f, maskAlpha, g_pConfigManager->getAnimationPropertyConfig("fade"), AVARDAMAGE_ENTIRE);

    HyprlandAPI::addConfigValue(pHandle, "plugin:hypro:monitor", Hyprlang::STRING{"eDP-1"});
    HyprlandAPI::addConfigValue(pHandle, "plugin:hypro:top", Hyprlang::INT{0});
    HyprlandAPI::addConfigValue(pHandle, "plugin:hypro:bottom", Hyprlang::INT{0});
    HyprlandAPI::addConfigValue(pHandle, "plugin:hypro:left", Hyprlang::INT{0});
    HyprlandAPI::addConfigValue(pHandle, "plugin:hypro:right", Hyprlang::INT{0});

    configHook = HyprlandAPI::registerCallbackDynamic(pHandle, "configReloaded", [&] (void* thisptr, SCallbackInfo& info, std::any data) { reloadConfig(); });
    HyprlandAPI::reloadConfig();

    tickHook = HyprlandAPI::registerCallbackDynamic(pHandle, "tick", [&] (void* thisptr, SCallbackInfo& info, std::any data) { onTick(); });
    renderHook = HyprlandAPI::registerCallbackDynamic(pHandle, "render", [&] (void* thisptr, SCallbackInfo& info, std::any data) { onRender(data); });

    return {"Hypro", "Irregular screen adaptation", "KZdkm", "0.1"};
}
