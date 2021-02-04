#include "CVolumeViewSync.h"

CVolumeViewSync::CVolumeViewSync(QObject *parent) : QObject(parent)
{
}

void CVolumeViewSync::syncView(CVolumeDisplay *pDisplay)
{
    assert(pDisplay);
}

void CVolumeViewSync::syncView(CVolumeDisplay *pDisplay1, CVolumeDisplay *pDisplay2)
{
    assert(pDisplay1);
    assert(pDisplay2);

    if(pDisplay1 == pDisplay2)
        return;

    //Synchronize image views
    m_imageViewSync.syncView(pDisplay1->getImageDisplay(), pDisplay2->getImageDisplay());
    setConnected(pDisplay1, pDisplay2, true);
}

void CVolumeViewSync::syncView(CVolumeDisplay *pVolumeDisplay, CImageDisplay *pImageDisplay)
{
    assert(pVolumeDisplay);
    assert(pImageDisplay);

    auto pVolumeImgDisplay = pVolumeDisplay->getImageDisplay();
    if(pVolumeImgDisplay == pImageDisplay || pVolumeImgDisplay == nullptr)
        return;

    //Synchronize image views
    m_imageViewSync.syncView(pVolumeDisplay->getImageDisplay(), pImageDisplay);
    setConnected(pVolumeDisplay, pImageDisplay, true);
}

void CVolumeViewSync::unsyncView(CVolumeDisplay *pVolumeDisplay, CImageDisplay *pImageDisplay)
{
    assert(pVolumeDisplay);
    assert(pImageDisplay);

    if(isConnected(pVolumeDisplay, pImageDisplay) == false)
        return;

    auto pVolumeImgDisplay = pVolumeDisplay->getImageDisplay();
    if(pVolumeImgDisplay != nullptr)
        m_imageViewSync.unsyncView(pVolumeImgDisplay, pImageDisplay);

    setConnected(pVolumeDisplay, pImageDisplay, false);
}

bool CVolumeViewSync::isConnected(CDataDisplay *pDisplay1, CDataDisplay *pDisplay2) const
{
    auto it = m_connections.find(std::make_pair(pDisplay1, pDisplay2));
    if(it != m_connections.end())
        return true;
    else
    {
        it = m_connections.find(std::make_pair(pDisplay2, pDisplay1));
        return (it != m_connections.end());
    }
}

void CVolumeViewSync::setConnected(CDataDisplay *pDisplay1, CDataDisplay *pDisplay2, bool bConnected)
{
    if(bConnected == true)
        m_connections.insert(std::make_pair(pDisplay1, pDisplay2));
    else
    {
        auto it = m_connections.find(std::make_pair(pDisplay1, pDisplay2));
        if(it != m_connections.end())
            m_connections.erase(it);
        else
        {
            it = m_connections.find(std::make_pair(pDisplay2, pDisplay1));
            if(it != m_connections.end())
                m_connections.erase(it);
        }
    }
}
