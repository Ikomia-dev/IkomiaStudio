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

#ifndef CWORKFLOWMODULEWIDGET_H
#define CWORKFLOWMODULEWIDGET_H

#include <QObject>
#include <QWidget>
#include "Main/forwards.hpp"
#include "CWorkflowView.h"

class QtVariantPropertyManager;
class QtTreePropertyBrowser;
class QtProperty;
class CProcessDocDlg;

class CWorkflowModuleWidget : public QWidget
{
    Q_OBJECT

    public:

        CWorkflowModuleWidget(QWidget* parent = nullptr);
        ~CWorkflowModuleWidget();

        void            setCurrentUser(const CUser& user);

        CWorkflowView*  getView() const;

    signals:

        void            doUpdateTitle();
        void            doRunFromActiveTask(const WorkflowTaskParamPtr& pParam);
        void            doNotifyWorkflowClosed();
        void            doSetGraphicsTool(GraphicsShape tool);
        void            doSetGraphicsCategory(const QString& category);
        void            doShowProcessPopup();
        void            doSendProcessAction(const WorkflowTaskPtr& pTask, int flag);

    public slots:

        void            onSetModel(CWorkflowManager* pModel);
        void            onAddTask(const WorkflowTaskPtr& pTask, const WorkflowVertex& id, const WorkflowVertex& parentId);
        void            onAddCandidateTask(const WorkflowTaskPtr& pTask, const WorkflowVertex& id);
        void            onSetScaleProjects(const QJsonArray& projects, const std::vector<QString>& namespaces);
        void            onUpdateTaskInfo(const WorkflowTaskPtr& pTask, const CTaskInfo &description);
        void            onUpdateTaskStateInfo(const VectorPairString& infoMap, const VectorPairString& customInfoMap);
        void            onWorkflowCreated();

    protected:

        void            hideEvent(QHideEvent* event) override;

    private slots:

        void            onNewWorkflow();
        void            onLoadWorkflow();
        void            onSaveWorkflow();
        void            onExportWorkflow();
        void            onPublishWorkflow();
        void            onCloseWorkflow();
        void            onShowProcessInfo();
        void            onIOPropertyValueChanged(QtProperty* pProperty, const QVariant& value);

    private:

        void            initLayout();
        void            initLeftTab();
        void            initTopTab();
        void            initConnections();

        QVBoxLayout*    createTab(QIcon icon, QString title, QWidget* pBtn);

        QToolButton*    addButtonToTop(const QSize& size, const QIcon& icon, const QString& tooltip, bool bCheckable = false);

        void            setTitle(const QString& title);
        void            setProcessInfoMap(const VectorPairString& infoMap, const VectorPairString& customInfoMap);
        void            setGroupProperties(QtTreePropertyBrowser *pPropList, const QString& title, const VectorPairString& properties);

        void            applyPropertyListStyle(QtTreePropertyBrowser *pPropList);

        void            remove(QLayout* pLayout);

        void            clearParamLayout();
        void            clearTabWidget();

        void            fillIOProperties(const WorkflowTaskPtr& taskPtr);
        void            fillProperty(QtProperty* pItem, const VectorPairString& properties);

        void            adjustProcessDocDlgPos();

    private:

        enum IOPropType { AUTO_SAVE, SAVE_FOLDER, SAVE_FORMAT };

        struct PropAttribute
        {
            IOPropType type;
            QVariant data;
        };

        QString                     m_name;
        CWorkflowView*              m_pView = nullptr;
        CRollupWidget*              m_pRollup = nullptr;
        QTabWidget*                 m_pTab = nullptr;
        CWorkflowManager*           m_pModel = nullptr;
        WorkflowTaskWidgetPtr       m_widgetPtr = nullptr;
        QVBoxLayout*                m_pParamLayout = nullptr;
        QVBoxLayout*                m_pInfoLayout = nullptr;
        QVBoxLayout*                m_pIOLayout = nullptr;
        QtTreePropertyBrowser*      m_pInfoPropertyList = nullptr;
        QtTreePropertyBrowser*      m_pIOPropertyList = nullptr;
        QtVariantPropertyManager*   m_pVariantManager = nullptr;
        QMap<QtProperty*, PropAttribute> m_ioProperties;
        CTaskInfo                   m_currentProcessInfo;
        CProcessDocDlg*             m_pProcessDocDlg = nullptr;
        QString                     m_lastPathWorkflow = "";
};

#endif // CWORKFLOWMODULEWIDGET_H
