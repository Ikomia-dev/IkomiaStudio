#ifndef CMULTIIMAGESDISPLAY_H
#define CMULTIIMAGESDISPLAY_H

#include "View/DoubleView/CDataDisplay.h"
#include "View/Common/CImageListView.h"
#include "Data/CMat.hpp"

class CMultiImageModel;
class CGraphicsLayer;


class CMultiImageDisplay : public CDataDisplay
{
    public:

        CMultiImageDisplay(QWidget* pParent = nullptr);

        void    setModel(CMultiImageModel* pModel);

    public slots:

        void    onShowImage(const QModelIndex &index);

    protected:

        void    keyPressEvent(QKeyEvent* event) override;

    private:

        void    initLayout();
        void    initConnections();

        void    loadImage(const QModelIndex &index);
        void    loadPreviousImage();
        void    loadNextImage();

        CMat    createDisplayOverlay(const CMat& img);

    private:

        CImageDisplay*  m_pImageDisplay = nullptr;
        CImageListView* m_pListView = nullptr;
        QStackedWidget* m_pStackedWidget = nullptr;
        CGraphicsLayer* m_pCurrentLayer = nullptr;
};

#endif // CMULTIIMAGESDISPLAY_H
