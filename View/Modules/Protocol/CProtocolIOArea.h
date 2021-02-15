/*
 * Copyright (C) 2021 Ikomia SAS
 * Contact: https://www.ikomia.com
 *
 * This file is part of the IkomiaStudio software.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CPROTOCOLIOAREA_H
#define CPROTOCOLIOAREA_H

#include "CProtocolLabelArea.h"
#include "Main/AppDefine.hpp"
#include "Core/CProtocol.h"
#include "IO/CImageProcessIO.h"
#include "CProtocolInputTypeDlg.h"

class CProtocolPortItem;
class CGraphicsDeletableButton;

class CProtocolIOArea : public QObject, public CProtocolLabelArea
{
    Q_OBJECT

    public:

        enum { Type = UserType + ProtocolGraphicsItem::IO_AREA };

        CProtocolIOArea(QString label, bool bInput, QGraphicsItem* parent = Q_NULLPTR);

        void                    setTaskId(const ProtocolVertex& id);
        void                    setProjectDataProxyModel(CProjectDataProxyModel* pModel);
        void                    setPortInfo(const CDataInfoPtr& info, int index);

        int                     type() const override;
        int                     getPortCount() const;
        int                     getConnectionCount() const;
        CProtocolPortItem*      getPort(int index) const;

        bool                    isInput() const;

        void                    activateActions(bool bActivate);

        void                    addPort(const ProtocolTaskIOPtr &inputPtr, bool isBatch);

        void                    updatePort(int index, const ProtocolTaskIOPtr& inputPtr, bool isBatch);

        void                    removePort(int index);

        void                    clear();

    signals:

        void                    doIOAreaChanged();
        void                    doGetIOInfo(const ProtocolVertex& taskId, int index, bool bInput);

    public slots:

        void                    onAddPort();

    protected:

        void                    hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
        void                    hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    private:

        QGraphicsProxyWidget*   createThumbnail(const std::shared_ptr<CProtocolTaskIO> &inputPtr, bool isBatch);
        void                    createInputTypeDlg();

        QPointF                 getNextPortCenter() const;
        QPixmap                 getThumbnailIcon(const std::shared_ptr<CProtocolTaskIO> &inputPtr, bool isBatch);
        QPixmap                 getImageIcon(const std::shared_ptr<CImageProcessIO>& inputImgPtr, bool isBatch);
        int                     getThumbnailIndex(QGraphicsProxyWidget* pThumbnail);

        QString                 createSvgThumbnails(QPixmap pixmap, int index);

        void                    changeInput(int index);

        void                    deleteInput(int index);

        void                    showAddPortWidget();

        void                    hideAddPortWidget();

    private:

        bool                            m_bInput = true;
        bool                            m_bActionsActivated = false;
        ProtocolVertex                  m_id;
        QVector<CProtocolPortItem*>     m_ports;
        QVector<QGraphicsProxyWidget*>  m_portThumbnails;
        QGraphicsProxyWidget*           m_pAddPortWidget = nullptr;
        CProtocolInputTypeDlg*          m_pInputChoiceDlg = nullptr;
        QSize                           m_thumbnailSize = QSize(40, 40);
        int                             m_portSpacing = 5;
};

#endif // CPROTOCOLIOAREA_H
