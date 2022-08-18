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

#ifndef APPDEFINE_HPP
#define APPDEFINE_HPP

#include "Main/CoreDefine.hpp"
#include "Workflow/CViewPropertyIO.h"

#define MATOMO_APP_ID   3

namespace Ikomia
{
    enum TreeItemType : size_t
    {
        NONE,
        PROJECT,
        FOLDER,
        DATASET,
        DIMENSION,
        IMAGE,
        PROCESS,
        GRAPHICS_LAYER,
        RESULT,
        VIDEO,
        LIVE_STREAM
    };

    enum class Relationship : size_t
    {
        ONE_TO_ONE,
        ONE_TO_MANY,
        MANY_TO_ONE,
        MANY_TO_MANY
    };

    enum WorkflowGraphicsItem
    {
        TASK,
        CONNECTION,
        PORT,
        IO_AREA,
        DUMMY,
        LABEL_AREA
    };

    enum WorkflowInputViewMode
    {
        ORIGIN,
        CURRENT
    };

    enum WorkflowInputMode : size_t
    {
        CURRENT_DATA,
        FIXED_DATA
    };

    enum Notification
    {
        INFO,
        WARNING,
        CRITICAL,
        DEBUG,
        EXCEPTION
    };

    enum DisplayType
    {
        EMPTY_DISPLAY,
        IMAGE_DISPLAY,
        MULTI_IMAGE_DISPLAY,
        VIDEO_DISPLAY,
        OPENGL_DISPLAY,
        VOLUME_DISPLAY,
        TABLE_DISPLAY,
        PLOT_DISPLAY,
        WIDGET_DISPLAY,
        TEXT_DISPLAY,
    };

    enum DisplayCategory
    {
        INPUT,
        OUTPUT
    };

    constexpr int _NotifDefaultDuration = 1500; //1.5s
    constexpr int _UserCheckFrequency = 600000; //10min

    constexpr auto _DefaultDirImg = "default_dir_img";
    constexpr auto _DefaultDirImgExport = "default_dir_img_export";
    constexpr auto _DefaultDirVideo = "default_dir_video";
    constexpr auto _DefaultDirVideoExport = "default_dir_video_export";
    constexpr auto _DefaultDirProject = "default_dir_project";
    constexpr auto _DefaultDirDicom = "default_dir_project_dicom";
    constexpr auto _DefaultDirProjectSaveAs = "default_dir_project_saveAs";
    constexpr auto _DefaultDirProjectExport = "default_dir_project_export";
    constexpr auto _defaultDirWorkflow = "default_dir_protocol";
    constexpr auto _defaultDirWorkflowExport = "default_dir_protocol_export";

    //Structure to manage loading of multiple data sources into a dataset
    using DatasetLoadPolicy = std::pair<Relationship, DataDimension>;
    //Structure to manage display type for outputs of protocols
    using OutputDisplays = std::map<DisplayType, std::vector<CViewPropertyIO*>>;
}

using namespace Ikomia;

#endif // APPDEFINE_HPP
