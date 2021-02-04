#ifndef CVIEWSYNC_H
#define CVIEWSYNC_H

#include <QObject>
#include <set>
#include "CImageDisplay.h"

class CImageViewSync : public QObject
{
    Q_OBJECT

    public:

        CImageViewSync();

        void    syncView(CImageDisplay* pImgDisplay1, CImageDisplay* pImgDisplay2);

        void    unsyncView(CImageDisplay *pImgDisplay);
        void    unsyncView(CImageDisplay* pImgDisplay1, CImageDisplay* pImgDisplay2);

    private:

        bool    isConnected(CImageDisplay* pImgDisplay1, CImageDisplay* pImgDisplay2) const;

        void    setConnected(CImageDisplay* pImgDisplay1, CImageDisplay* pImgDisplay2, bool bConnected);

    private:

        std::set<std::pair<CImageDisplay*, CImageDisplay*>> m_connections;
};

#endif // CVIEWSYNC_H
