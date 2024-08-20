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

#ifndef CGRAPHICSLAYERINFO_HPP
#define CGRAPHICSLAYERINFO_HPP

#include "Graphics/CGraphicsLayer.h"

class CGraphicsLayerInfo
{
    public:

        enum DisplayTarget { SOURCE, RESULT };

        CGraphicsLayerInfo(){}
        CGraphicsLayerInfo(CGraphicsLayer* pLayer, int refImgDisplayIndex, DisplayType refImgDisplayType, int displayTarget, bool bTopMost)
        {
            m_pLayer = pLayer;
            m_refImageDisplayIndex = refImgDisplayIndex;
            m_refImageDisplayType = refImgDisplayType;
            m_displayTarget = displayTarget;
            m_bTopMost = bTopMost;
        }
        CGraphicsLayerInfo(CGraphicsLayer* pLayer, int refImgDisplayIndex, DisplayType refImgDisplayType, int displayTarget)
        {
            m_pLayer = pLayer;
            m_refImageDisplayIndex = refImgDisplayIndex;
            m_refImageDisplayType = refImgDisplayType;
            m_displayTarget = displayTarget;
        }

        bool operator==(const CGraphicsLayerInfo& info) const
        {
            return (m_pLayer == info.m_pLayer &&
                    m_refImageDisplayIndex == info.m_refImageDisplayIndex &&
                    m_refImageDisplayType == info.m_refImageDisplayType &&
                    m_displayTarget == info.m_displayTarget);
        }

        void    deleteLayer()
        {
            m_pLayer->deleteChildItems();
            delete m_pLayer;
            m_pLayer = nullptr;
        }

    public:

        //Weak pointer
        CGraphicsLayer* m_pLayer = nullptr;
        int             m_refImageDisplayIndex = 0;
        DisplayType     m_refImageDisplayType = DisplayType::IMAGE_DISPLAY;
        int             m_displayTarget = SOURCE;
        bool            m_bTopMost = false;
};

#endif // CGRAPHICSLAYERINFO_HPP
