#pragma once
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>

struct App : winrt::implements<App, winrt::Windows::ApplicationModel::Core::IFrameworkView>
{
    void Initialize(winrt::Windows::ApplicationModel::Core::CoreApplicationView const& view);
    void SetWindow(winrt::Windows::UI::Core::CoreWindow const& window);
    void Load(winrt::hstring const& entryPoint);
    void Run();
    void Uninitialize();

private:
    winrt::Windows::UI::Core::CoreWindow m_window{ nullptr };
    bool m_windowClosed{ false };
    bool m_windowVisible{ true };
};
