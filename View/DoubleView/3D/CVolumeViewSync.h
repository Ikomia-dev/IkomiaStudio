#ifndef CVOLUMEVIEWSYNC_H
#define CVOLUMEVIEWSYNC_H

#include <QObject>
#include "CVolumeDisplay.h"
#include "View/DoubleView/Image/CImageViewSync.h"

class CVolumeViewSync : public QObject
{
    Q_OBJECT

    public:

        explicit CVolumeViewSync(QObject *parent = nullptr);

        void    syncView(CVolumeDisplay* pDisplay);
        void    syncView(CVolumeDisplay* pDisplay1, CVolumeDisplay* pDisplay2);
        void    syncView(CVolumeDisplay* pVolumeDisplay, CImageDisplay* pImageDisplay);

        void    unsyncView(CVolumeDisplay* pVolumeDisplay, CImageDisplay* pImageDisplay);

    private:

        bool    isConnected(CDataDisplay* pDisplay1, CDataDisplay* pDisplay2) const;

        void    setConnected(CDataDisplay* pDisplay1, CDataDisplay* pDisplay2, bool bConnected);

    private:

        CImageViewSync  m_imageViewSync;
        std::set<std::pair<CDataDisplay*, CDataDisplay*>>   m_connections;
};

#endif // CVOLUMEVIEWSYNC_H
