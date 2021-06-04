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

#ifndef CWORKFLOWINPUTVIEWMANAGER_H
#define CWORKFLOWINPUTVIEWMANAGER_H

#include <QObject>
#include "Workflow/CWorkflowTask.h"
#include "View/DoubleView/CDataViewer.h"
#include "CWorkflowInput.h"

class CImageScene;
class CProjectManager;

class CWorkflowInputViewManager : public QObject
{
    Q_OBJECT

    public:

        CWorkflowInputViewManager(CWorkflowInputs* pInputs, WorkflowInputViewMode mode);

        void    setManagers(CProjectManager *pProjectMgr);
        void    setViewMode(WorkflowInputViewMode mode);
        void    setLiveInputIndex(size_t index);

        void    manageOriginVideoInput(size_t index);
        void    manageInputs(const WorkflowTaskPtr &pTask);

        void    clear(const WorkflowTaskPtr &pTask);

    signals:

        void    doInitDisplay(const std::map<DisplayType, int>& mapTypeCount);
        void    doDisplayImage(int index, CImageScene *pScene, QImage image, const QString& name, CViewPropertyIO* pViewProperty);
        void    doDisplayVideo(const QModelIndex& modelIndex, int index, CImageScene *pScene, QImage image, const QString& name, bool bVideoChanged, CViewPropertyIO* pViewProperty);
        void    doDisplayVideoImage(const QModelIndex& modelIndex, int inputIndex);
        void    doUpdateVolumeImage(int index, QImage image, const QString& name, CViewPropertyIO* pViewProperty);
        void    doApplyViewProperty();
        void    doInitVideoInfo(const QModelIndex& modelIndex, int index);

    private:

        std::map<DisplayType,int> getInputTypes(const WorkflowTaskPtr &pTask) const;
        std::map<DisplayType,int> getInputType(const WorkflowTaskPtr &pTask, int index) const;
        DisplayType               getDataViewerType(const IODataType &dataType) const;
        QModelIndex               getSrcModelIndex(size_t index) const;

        void    manageImageInput(int index, const WorkflowTaskIOPtr& inputPtr, const std::string& taskName, CViewPropertyIO *pViewProp);
        void    manageVideoInput(int index, const WorkflowTaskIOPtr& inputPtr, const std::string& taskName, CViewPropertyIO* pViewProp);
        void    manageVolumeInput(int index, const WorkflowTaskIOPtr& inputPtr, const std::string& taskName, CViewPropertyIO *pViewProp);
        void    manageGraphicsInput(const WorkflowTaskIOPtr& inputPtr);

    private:

        CProjectManager*        m_pProjectMgr = nullptr;
        CWorkflowInputs*        m_pInputs = nullptr;
        WorkflowInputViewMode   m_viewMode = WorkflowInputViewMode::ORIGIN;
        size_t                  m_videoInputIndex = 0;
};

#endif // CWORKFLOWINPUTVIEWMANAGER_H
