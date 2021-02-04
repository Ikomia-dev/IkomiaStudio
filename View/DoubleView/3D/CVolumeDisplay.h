#ifndef CDATAVOLUMEVIEW_H
#define CDATAVOLUMEVIEW_H

#include <QWidget>
#include "../CDataDisplay.h"
#include "Main/forwards.hpp"

class CVolumeDisplay : public CDataDisplay
{
    Q_OBJECT

    public:

        CVolumeDisplay(QWidget* pParent = nullptr);

        void            setImage(CImageScene* pScene, QImage image, const QString& name);
        virtual void    setViewSpaceShared(bool bShared) override;
        virtual void    setSelected(bool bSelect) override;

        void            initGL();

        void            changeVolume();

        void            updateRenderVolume();

        CImageDisplay*  getImageDisplay();
        C3dDisplay*     get3dDisplay();

    protected:

        bool            eventFilter(QObject* obj, QEvent* event) override;

    private:

        void            initLayout();
        void            initConnections();

    private:

        QSplitter*      m_pSplitter = nullptr;
        CImageDisplay*  m_pView2D = nullptr;
        C3dDisplay*     m_pView3D = nullptr;
};

#endif // CDATAVOLUME_H
