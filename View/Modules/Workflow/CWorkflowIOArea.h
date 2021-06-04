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

#ifndef CWORKFLOWIOAREA_H
#define CWORKFLOWIOAREA_H

#include "CWorkflowLabelArea.h"
#include "Main/AppDefine.hpp"
#include "Core/CWorkflow.h"
#include "IO/CImageIO.h"
#include "CWorkflowInputTypeDlg.h"

class CWorkflowPortItem;
class CGraphicsDeletableButton;

class CWorkflowIOArea : public QObject, public CWorkflowLabelArea
{
    Q_OBJECT

    public:

        enum { Type = UserType + WorkflowGraphicsItem::IO_AREA };

        CWorkflowIOArea(QString label, bool bInput, QGraphicsItem* parent = Q_NULLPTR);

        void                    setTaskId(const WorkflowVertex& id);
        void                    setProjectDataProxyModel(CProjectDataProxyModel* pModel);
        void                    setPortInfo(const CDataInfoPtr& info, int index);

        int                     type() const override;
        int                     getPortCount() const;
        int                     getConnectionCount() const;
        CWorkflowPortItem*      getPort(int index) const;

        bool                    isInput() const;

        void                    activateActions(bool bActivate);

        void                    addPort(const WorkflowTaskIOPtr &inputPtr, bool isBatch);

        void                    updatePort(int index, const WorkflowTaskIOPtr& inputPtr, bool isBatch);

        void                    removePort(int index);

        void                    clear();

    signals:

        void                    doIOAreaChanged();
        void                    doGetIOInfo(const WorkflowVertex& taskId, int index, bool bInput);

    public slots:

        void                    onAddPort();

    protected:

        void                    hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
        void                    hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    private:

        QGraphicsProxyWidget*   createThumbnail(const std::shared_ptr<CWorkflowTaskIO> &inputPtr, bool isBatch);
        void                    createInputTypeDlg();

        QPointF                 getNextPortCenter() const;
        QPixmap                 getThumbnailIcon(const std::shared_ptr<CWorkflowTaskIO> &inputPtr, bool isBatch);
        QPixmap                 getImageIcon(const std::shared_ptr<CImageIO>& inputImgPtr, bool isBatch);
        int                     getThumbnailIndex(QGraphicsProxyWidget* pThumbnail);

        QString                 createSvgThumbnails(QPixmap pixmap, int index);

        void                    changeInput(int index);

        void                    deleteInput(int index);

        void                    showAddPortWidget();

        void                    hideAddPortWidget();

    private:

        bool                            m_bInput = true;
        bool                            m_bActionsActivated = false;
        WorkflowVertex                  m_id;
        QVector<CWorkflowPortItem*>     m_ports;
        QVector<QGraphicsProxyWidget*>  m_portThumbnails;
        QGraphicsProxyWidget*           m_pAddPortWidget = nullptr;
        CWorkflowInputTypeDlg*          m_pInputChoiceDlg = nullptr;
        QSize                           m_thumbnailSize = QSize(40, 40);
        int                             m_portSpacing = 5;
};

#endif // CWORKFLOWIOAREA_H
