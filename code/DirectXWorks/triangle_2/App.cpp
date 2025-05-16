#include "pch.h"
#include "Triangle.h"

// Use some common namespaces to simplify the code
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Popups;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Platform;

// the class definition for the core "framework" of our app
ref class App sealed : public IFrameWorkView
{
	bool WindowClosed;
	CTriangle CTriangle; // a class to store our triangle code

	virtual void Initialize();
	virtual void SetWindow();
	virtual void Load();
	virtual void Run();
	virtual void Uninitialize();
	virtual void OnActivated();
	virtual void Closed();
	void Suspending();
	void Resuming();
}

// the class definition that creates our core framework
ref class AppSource sealed : IFrameworkViewSource
{
public:
    virtual IFrameworkView^ CreateView()
    {
        return ref new App();    // create the framework view and return it
    }
};

[MTAThread]    // define main() as a multi-threaded-apartment function


int main(Array<String^>^ args)
{
    CoreApplication::Run(ref new AppSource());    // run the framework view source
    return 0;
}