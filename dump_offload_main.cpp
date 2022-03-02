#include "dump_dbus_util.hpp"
#include "dump_offload_mgr.hpp"

#include <fmt/format.h>

#include <cstdlib>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>

using ::phosphor::logging::level;
using ::phosphor::logging::log;

int main()
{
    try
    {
        auto bus = sdbusplus::bus::new_default();
        auto event = sdeventplus::Event::get_default();
        // Changing a system from hmc-managed to non-hmc manged is a disruptive
        // process (Power off the system, do some clean ups and IPL).
        // Changing a system from non-hmc managed to hmc-manged can be done at
        // runtime.
        // Not creating offloader objects if system is HMC managed
        // TODO #https://github.com/ibm-openbmc/powervm-handler/issues/8
        if (openpower::dump::isSystemHMCManaged(bus))
        {
            log<level::ERR>("HMC managed system exiting the application");
            return 0;
        }
        log<level::INFO>("Non HMC managed system initiating dump offloads");

        if (!openpower::dump::isHostRunning(bus))
        {
            log<level::ERR>("Host is not in running state, offload will start "
                            "when Host changes to running state");
        }

        openpower::dump::DumpOffloadManager manager(bus);
        manager.offload();
        bus.attach_event(event.get(), SD_EVENT_PRIORITY_NORMAL);
        return event.loop();
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>(
            fmt::format("exception during application load({})", ex.what())
                .c_str());
        throw;
    }
}
