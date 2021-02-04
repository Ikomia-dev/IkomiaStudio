#include "CProgressCircleManager.h"
#include <QApplication>
#include <QParallelAnimationGroup>

CProgressCircleManager::CProgressCircleManager()
{
}

CProgressCircle* CProgressCircleManager::createProgress(CProgressSignalHandler* pSignal, bool bMainThread)
{
    CProgressCircle* pProgress = new CProgressCircle(pSignal, bMainThread);
    pProgress->setFixedSize(m_width, m_height);
    pProgress->setColorBase(m_colorBase);
    pProgress->setColorBg(m_colorBg);
    pProgress->setColorInner(m_colorInner);
    return pProgress;
}

CProgressCircle *CProgressCircleManager::createInfiniteProgress(bool bMainThread)
{
    m_pInfiniteProgress = new CProgressCircle(nullptr, bMainThread);
    m_pInfiniteProgress->setFixedSize(m_width, m_height);
    m_pInfiniteProgress->setColorBase(m_colorBase);
    m_pInfiniteProgress->setColorBg(m_colorBg);
    m_pInfiniteProgress->setColorInner(m_colorInner);

    return m_pInfiniteProgress;
}

void CProgressCircleManager::setProgressSize(int w, int h)
{
    m_width = w;
    m_height = h;
}

void CProgressCircleManager::setColorBase(const QColor& color)
{
    m_colorBase = color;
}

void CProgressCircleManager::setColorBg(const QColor& color)
{
    m_colorBg = color;
}

void CProgressCircleManager::setColorInner(const QColor& color)
{
    m_colorInner = color;
}
