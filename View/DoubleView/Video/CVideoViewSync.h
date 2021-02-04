#ifndef CVIDEOVIEWSYNC_H
#define CVIDEOVIEWSYNC_H

#include <QObject>
#include <set>
#include "CVideoDisplay.h"
#include "View/DoubleView/Image/CImageViewSync.h"

class CVideoViewSync : public QObject
{
    Q_OBJECT

    public:

        CVideoViewSync();

        void    syncView(CVideoDisplay* pVideoDisplay1, CVideoDisplay* pVideoDisplay2);

        void    unsyncView(CVideoDisplay* pVideoDisplay);
        void    unsyncView(CVideoDisplay* pVideoDisplay1, CVideoDisplay* pVideoDisplay2);

        void    syncSliderLength(CVideoDisplay* pSrcDisplay, int length);
        void    syncSliderPos(CVideoDisplay* pSrcDisplay, int pos);
        void    syncTotalTime(CVideoDisplay* pSrcDisplay, int time);
        void    syncCurrentTime(CVideoDisplay* pSrcDisplay, int time);
        void    syncFps(CVideoDisplay* pSrcDisplay, double fps);
        void    syncStreamOptions(CVideoDisplay* pSrcDisplay, bool bIsStream);

    private:

        bool    isConnected(CVideoDisplay* pVideoDisplay1, CVideoDisplay* pVideoDisplay2) const;

        void    setConnected(CVideoDisplay* pVideoDisplay1, CVideoDisplay* pVideoDisplay2, bool bConnected);

    private:

        CImageViewSync  m_imageViewSync;
        std::set<std::pair<CVideoDisplay*, CVideoDisplay*>> m_connections;
};
#endif // CVIDEOVIEWSYNC_H
