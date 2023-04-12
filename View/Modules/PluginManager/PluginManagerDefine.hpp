// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef PLUGINMAKERDEFINE_HPP
#define PLUGINMAKERDEFINE_HPP

namespace Ikomia
{
    namespace PluginManager
    {
        enum ProcessBaseClass
        {
            CWORKFLOW_TASK,
            C2DIMAGE_TASK,
            C2DIMAGE_INTERACTIVE_TASK,
            CVIDEO_TASK,
            CCLASSIFICATION_TASK,
            COBJECT_DETECTION_TASK,
            CSEMANTIC_SEGMENTATION_TASK,
            CINSTANCE_SEGMENTATION_TASK,
            CKEYPOINT_DETECTION_TASK,
            CVIDEO_OF_TASK
        };

        enum WidgetBaseClass
        {
            CWORKFLOW_TASK_WIDGET
        };
    }
}

#endif // PLUGINMAKERDEFINE_HPP
