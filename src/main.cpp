#include "core/global/globals.hpp"
#include "ui/impl/webview/webview.hpp"
#include <InstanceGuard.hpp>
#include <fancy.hpp>

#if defined(_WIN32)
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main()
#endif
{
    InstanceGuard::InstanceGuard guard("soundux-guard");
    if (guard.IsAnotherInstanceRunning())
    {
        Fancy::fancy.logTime().failure() << "Another Instance is already running!" << std::endl;
        std::terminate();
    }

#if defined(__linux__)
    if (!Soundux::Globals::gPulse.isSwitchOnConnectLoaded())
    {
        Soundux::Globals::gPulse.setup();
    }
    Soundux::Globals::gAudio.setup();
#endif
    Soundux::Globals::gConfig.load();
#if defined(__linux__)
    if (Soundux::Globals::gConfig.settings.useAsDefaultDevice)
    {
        Soundux::Globals::gPulse.setDefaultSourceToSoundboardSink();
    }
#endif
    Soundux::Globals::gData = Soundux::Globals::gConfig.data;
    Soundux::Globals::gSettings = Soundux::Globals::gConfig.settings;

    Soundux::Globals::gGui = std::make_unique<Soundux::Objects::WebView>();
    Soundux::Globals::gGui->setup();
    Soundux::Globals::gGui->mainLoop();

    Soundux::Globals::gAudio.destory();
#if defined(__linux__)
    Soundux::Globals::gPulse.destroy();
#endif
    Soundux::Globals::gConfig.data = Soundux::Globals::gData;
    Soundux::Globals::gConfig.settings = Soundux::Globals::gSettings;
    Soundux::Globals::gConfig.save();

    return 0;
}