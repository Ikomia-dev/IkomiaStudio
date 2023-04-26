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

#ifndef CPLUGINSTOREDLG_H
#define CPLUGINSTOREDLG_H

#include <QDialog>
#include "View/Common/CDialog.h"
#include "Task/CTaskInfo.h"
#include "Model/User/CUser.h"
#include "Model/Store/CPluginModel.h"

class CStorePluginListView;
class CStoreQueryModel;
class CProcessDocWidget;

class CStoreDlg : public CDialog
{
    Q_OBJECT

    public:

        CStoreDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void            setCurrentUser(const CUser& user);

    signals:

        void            doGetHubModel();
        void            doGetWorkspaceModel();
        void            doGetLocalModel();
        void            doPublishPlugin(const QModelIndex& index);
        void            doInstallHubPlugin(const QModelIndex& index);
        void            doInstallWorkspacePlugin(const QModelIndex& index);
        void            doUpdatePluginInfo(bool bFullEdit, const CTaskInfo& info);
        void            doServerSearchChanged(const QString& text);
        void            doLocalSearchChanged(const QString& text);
        void            doClose();

    public slots:

        void            onSetPluginModel(CPluginModel* pModel);
        void            onShowLocalPluginInfo(const QModelIndex& index);
        void            onShowServerPluginInfo(const QModelIndex& index);

    protected:

        virtual void    showEvent(QShowEvent* event) override;
        virtual void    closeEvent(QCloseEvent* event) override;
        virtual void    hideEvent(QHideEvent* event) override;

    private:

        void            initLayout();
        void            initConnections();

        QWidget*        createLeftWidget();
        QWidget*        createRightWidget();
        QWidget*        createPluginsView(CPluginModel::Type type);
        QLabel*         createMessageLabel(const QString &msg);

        void            showProcessInfo(const QModelIndex& index);

    private:

        QFrame*                 m_pTitleBar = nullptr;
        QStackedWidget*         m_pPluginStackWidget = nullptr;
        QStackedWidget*         m_pRightStackWidget = nullptr;
        QPushButton*            m_pBtnHub = nullptr;
        QPushButton*            m_pBtnWorkspace = nullptr;
        QPushButton*            m_pBtnLocalPlugins = nullptr;
        QPushButton*            m_pBtnHubRefresh = nullptr;
        QPushButton*            m_pBtnWorkspaceRefresh = nullptr;
        QLineEdit*              m_pEditHubSearch = nullptr;
        QLineEdit*              m_pEditWorkspaceSearch = nullptr;
        QLineEdit*              m_pEditLocalSearch = nullptr;
        QLabel*                 m_pLabelMsgHub = nullptr;
        QLabel*                 m_pLabelMsgWorkspace = nullptr;
        QLabel*                 m_pLabelMsgLocal = nullptr;
        CStorePluginListView*   m_pHubView = nullptr;
        CStorePluginListView*   m_pWorkspaceView = nullptr;
        CStorePluginListView*   m_pLocalView = nullptr;
        CProcessDocWidget*      m_pDocWidget = nullptr;
};

#endif // CPLUGINSTOREDLG_H
