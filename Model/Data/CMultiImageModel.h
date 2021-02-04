#ifndef CMULTIIMAGEMODEL_H
#define CMULTIIMAGEMODEL_H

#include <QAbstractListModel>
#include "Graphics/CGraphicsLayer.h"
#include "DesignPattern/ctpl_stl.h"


class CMultiImageModel: public QAbstractListModel
{
    public:

        CMultiImageModel(QObject *parent = nullptr);

        void            addImage(const QString& path, const QString& overlayPath, const std::vector<ProxyGraphicsItemPtr>& graphics);

        void            setGraphicsContext(const GraphicsContextPtr& contextPtr);

        QString         getPath(int index) const;
        QString         getOverlayPath(int index) const;
        QString         getFileName(int index) const;
        CGraphicsLayer* getGraphicsLayer(int index) const;

    protected:

        int             rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant        data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant        headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    private:

        struct ImageData
        {
            QString         m_path = "";
            QString         m_filename = "";
            QString         m_overlayPath = "";
            CGraphicsLayer* m_pGraphicsLayer = nullptr;
            QPixmap         m_pixmap;
        };

        std::vector<ImageData>  m_data;
        GraphicsContextPtr      m_graphicsContextPtr = nullptr;
        ctpl::thread_pool       m_pool;
};

#endif // CMULTIIMAGEMODEL_H
