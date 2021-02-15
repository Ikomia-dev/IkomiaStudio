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
