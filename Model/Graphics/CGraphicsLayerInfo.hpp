#ifndef CGRAPHICSLAYERINFO_HPP
#define CGRAPHICSLAYERINFO_HPP

#include "Graphics/CGraphicsLayer.h"

class CGraphicsLayerInfo
{
    public:

        enum DisplayTarget { SOURCE, RESULT };

        CGraphicsLayerInfo(){}
        CGraphicsLayerInfo(CGraphicsLayer* pLayer, int imageIndex, int displayTarget, bool bTopMost)
        {
            m_pLayer = pLayer;
            m_imageIndex = imageIndex;
            m_displayTarget = displayTarget;
            m_bTopMost = bTopMost;
        }
        CGraphicsLayerInfo(CGraphicsLayer* pLayer, int imageIndex, int displayTarget)
        {
            m_pLayer = pLayer;
            m_imageIndex = imageIndex;
            m_displayTarget = displayTarget;
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
        int             m_imageIndex = 0;
        int             m_displayTarget = SOURCE;
        bool            m_bTopMost = false;
};

#endif // CGRAPHICSLAYERINFO_HPP
