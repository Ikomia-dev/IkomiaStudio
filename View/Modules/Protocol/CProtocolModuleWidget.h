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

#ifndef CPROTOCOLMODULEWIDGET_H
#define CPROTOCOLMODULEWIDGET_H

#include <QObject>
#include <QWidget>
#include "Main/forwards.hpp"
#include "CProtocolView.h"

class QtVariantPropertyManager;
class QtTreePropertyBrowser;
class QtProperty;
class CProcessDocDlg;

class CProtocolModuleWidget : public QWidget
{
    Q_OBJECT

    public:

        CProtocolModuleWidget(QWidget* parent = nullptr);
        ~CProtocolModuleWidget();

        void            setCurrentUser(const CUser& user);

        CProtocolView*  getView() const;

    signals:

        void            doUpdateTitle();
        void            doRunFromActiveTask(const ProtocolTaskParamPtr& pParam);
        void            doNotifyProtocolClosed();
        void            doSetGraphicsTool(GraphicsShape tool);
        void            doSetGraphicsCategory(const QString& category);
        void            doShowProcessPopup();
        void            doSendProcessAction(const ProtocolTaskPtr& pTask, int flag);

    public slots:

        void            onSetModel(CProtocolManager* pModel);
        void            onAddTask(const ProtocolTaskPtr& pTask, const ProtocolVertex& id, const ProtocolVertex& parentId);
        void            onAddCandidateTask(const ProtocolTaskPtr& pTask, const ProtocolVertex& id);
        void            onUpdateTaskInfo(const ProtocolTaskPtr& pTask, const CProcessInfo &description);
        void            onUpdateTaskStateInfo(const VectorPairString& infoMap, const VectorPairString& customInfoMap);
        void            onProtocolCreated();
        void            onSaveProtocol();
        void            onExportProtocol();
        void            onLoadProtocol();
        void            onCloseProtocol();
        void            onNewProtocol();

    protected:

        void            hideEvent(QHideEvent* event) override;

    private slots:

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

        void            fillIOProperties(const ProtocolTaskPtr& taskPtr);
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
        CProtocolView*              m_pView = nullptr;
        CRollupWidget*              m_pRollup = nullptr;
        QTabWidget*                 m_pTab = nullptr;
        CProtocolManager*           m_pModel = nullptr;
        ProtocolTaskWidgetPtr       m_widgetPtr = nullptr;
        QVBoxLayout*                m_pParamLayout = nullptr;
        QVBoxLayout*                m_pInfoLayout = nullptr;
        QVBoxLayout*                m_pIOLayout = nullptr;
        QtTreePropertyBrowser*      m_pInfoPropertyList = nullptr;
        QtTreePropertyBrowser*      m_pIOPropertyList = nullptr;
        QtVariantPropertyManager*   m_pVariantManager = nullptr;
        QMap<QtProperty*, PropAttribute> m_ioProperties;
        CProcessInfo                m_currentProcessInfo;
        CProcessDocDlg*             m_pProcessDocDlg = nullptr;
        QString                     m_lastPathProtocol = "";
};

#endif // CPROTOCOLMODULEWIDGET_H
