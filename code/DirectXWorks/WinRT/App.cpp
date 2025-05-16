#include "App.h"
//#include <winrt/Windows.ApplicationModel.Core.h>
//#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.Foundation.h>

using namespace winrt;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;
using namespace Windows::Foundation;

void App::Initialize(CoreApplicationView const&)
{
    // Qui puoi registrare eventi di attivazione o altro
}

void App::SetWindow(CoreWindow const& window)
{
    m_window = window;

    m_window.VisibilityChanged({ this, [](auto const&, VisibilityChangedEventArgs const& args)
    {
        m_windowVisible = args.Visible();
    }});

    m_window.Closed({ this, [](auto const&, CoreWindowEventArgs const&)
    {
        m_windowClosed = true;
    }});
}

void App::Load(hstring const&)
{
    // Carica risorse dell'app
}

void App::Run()
{
    m_window.Activate(); // Rende la finestra visibile

    while (!m_windowClosed)
    {
        if (m_windowVisible)
        {
            CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
        }
        else
        {
            CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
        }
    }
}

void App::Uninitialize()
{
    // Cleanup finale
}