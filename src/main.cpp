#include <hyprland/src/plugins/PluginSystem.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/Compositor.hpp>

inline HANDLE pHandle;

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

std::string monitor = "eDP-1";
int top = 0;
int bottom = 0;
int left = 0;
int right = 0;

CAnimatedVariable<float> maskAlpha;

void onTick() {
    for (auto& w : g_pCompositor->m_vWindows) {
        if (w->m_bIsFullscreen && w->m_pWorkspace->m_efFullscreenMode == FULLSCREEN_FULL) {
            const auto m = g_pCompositor->getMonitorFromID(w->m_iMonitorID);
            if (m != g_pCompositor->getMonitorFromString(monitor)) continue;
            const Vector2D pos = {m->vecPosition.x + (left / m->scale), m->vecPosition.y + (top / m->scale)};
            const Vector2D size = {m->vecSize.x - (left / m->scale) - (right / m->scale), m->vecSize.y - (top / m->scale) - (bottom / m->scale)};
            if (w->m_vRealPosition.goal() != pos)
                w->m_vRealPosition = pos;
            if (w->m_vRealSize.goal() != size)
                w->m_vRealSize = size;
        }
    }

}

void onRender(std::any args) {
    const auto renderStage = std::any_cast<eRenderStage>(args);

    if (renderStage == eRenderStage::RENDER_PRE_WINDOWS) {
        const auto m = g_pCompositor->getMonitorFromName(monitor);
        if (g_pHyprOpenGL->m_RenderData.pMonitor == m) {
            if (m->activeWorkspace) {
                if (m->activeWorkspace->m_bHasFullscreenWindow && m->activeWorkspace->m_efFullscreenMode == FULLSCREEN_FULL && maskAlpha.goal() != 1.f) {
                    maskAlpha = 1.f;
                }
                else if (!(m->activeWorkspace->m_bHasFullscreenWindow && m->activeWorkspace->m_efFullscreenMode == FULLSCREEN_FULL) && maskAlpha.goal() != 0.f) {
                    maskAlpha = 0.f;
                }
            }
            CBox monBox = CBox(m->vecPosition, m->vecTransformedSize);
            if (maskAlpha.value() > 0.f && maskAlpha.isBeingAnimated())
                g_pHyprRenderer->damageMonitor(m);
            g_pHyprOpenGL->renderRect(&monBox, CColor(0, 0, 0, maskAlpha.value()));
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

    maskAlpha.create(g_pConfigManager->getAnimationPropertyConfig("fade"), AVARDAMAGE_ENTIRE);

    HyprlandAPI::addConfigValue(pHandle, "plugin:hypro:monitor", Hyprlang::STRING{"eDP-1"});
    HyprlandAPI::addConfigValue(pHandle, "plugin:hypro:top", Hyprlang::INT{0});
    HyprlandAPI::addConfigValue(pHandle, "plugin:hypro:bottom", Hyprlang::INT{0});
    HyprlandAPI::addConfigValue(pHandle, "plugin:hypro:left", Hyprlang::INT{0});
    HyprlandAPI::addConfigValue(pHandle, "plugin:hypro:right", Hyprlang::INT{0});

    HyprlandAPI::registerCallbackDynamic(pHandle, "configReloaded", [&] (void* thisptr, SCallbackInfo& info, std::any data) { reloadConfig(); });
    HyprlandAPI::reloadConfig();

    HyprlandAPI::registerCallbackDynamic(pHandle, "tick", [&] (void* thisptr, SCallbackInfo& info, std::any data) { onTick(); });
    HyprlandAPI::registerCallbackDynamic(pHandle, "render", [&] (void* thisptr, SCallbackInfo& info, std::any data) { onRender(data); });

    return {"Hypro", "Irregular screen adaptation", "KZdkm", "0.1"};
}