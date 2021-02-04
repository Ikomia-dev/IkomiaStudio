#ifndef PLUGINMAKERDEFINE_HPP
#define PLUGINMAKERDEFINE_HPP

namespace Ikomia
{
    namespace PluginManager
    {
        enum ProcessBaseClass
        {
            CPROTOCOL_TASK,
            CIMAGE_PROCESS_2D,
            CINTERACTIVE_IMAGE_PROCESS_2D,
            CVIDEO_PROCESS,
            CVIDEO_PROCESS_OF
        };

        enum WidgetBaseClass
        {
            CPROTOCOL_TASK_WIDGET
        };
    }
}

#endif // PLUGINMAKERDEFINE_HPP
