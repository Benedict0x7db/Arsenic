#include <Arsenic/Arsenic.hpp>

#include "SandboxLayer.hpp"

namespace arsenic
{
    class SandboxApp : public Application
    {
    public:
        SandboxApp() : Application("SandboxApp")
        {
            pushLayer<SandboxLayer>();   
        }
    };

    UniquePtr<Application> createApplication()
    {
        return createUniquePtr<SandboxApp>();
    }
}