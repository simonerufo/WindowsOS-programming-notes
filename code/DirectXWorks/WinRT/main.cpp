#include "App.h"
#include <winrt/Windows.ApplicationModel.Core.h>

using namespace winrt;
using namespace Windows::ApplicationModel::Core;

struct AppSource : implements<AppSource, IFrameworkViewSource>
{
    IFrameworkView CreateView()
    {
        return make<App>();
    }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    winrt::init_apartment();
    CoreApplication::Run(make<AppSource>());
    return 0;
}