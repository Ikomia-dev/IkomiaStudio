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

#ifndef CHUBDLG_H
#define CHUBDLG_H

#include <QDialog>
#include "View/Common/CDialog.h"
#include "Task/CTaskInfo.h"
#include "Model/User/CUser.h"
#include "Model/Hub/CPluginModel.h"

class CHubPluginListView;
class CHubQueryModel;
class CProcessDocWidget;

class CHubDlg : public CDialog
{
    Q_OBJECT

    enum class ModelRequestStage : int
    {
        IDLE,
        HUB_SENT,
        HUB_DONE,
        WORKSPACE_SENT,
        WORKSPACE_DONE,
        LOCAL_SENT,
        LOCAL_DONE,
    };

    public:

        CHubDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void            setCurrentUser(const CUser& user);

    signals:

        void            doGetHubModel();
        void            doGetWorkspaceModel();
        void            doGetLocalModel();
        void            doGetNextPublishInfo(const QModelIndex& index);
        void            doPublishHub(const QModelIndex& index, const QJsonObject& info);
        void            doPublishWorkspace(const QModelIndex& index, const QString& workspace="");
        void            doInstallPlugin(CPluginModel::Type type, const QModelIndex& index);
        void            doHubSearchChanged(const QString& text);
        void            doWorkspaceSearchChanged(const QString& text);
        void            doLocalSearchChanged(const QString& text);
        void            doClose();

    public slots:

        void            onSetPluginModel(CPluginModel* pModel);
        void            onModelError(CPluginModel* pModel);
        void            onSetNextPublishInfo(const QModelIndex &index, const QJsonObject &publishInfo);
        void            onShowPluginInfo(const QModelIndex &index);
        void            onPublishPluginToWorkspace(const QModelIndex &index);

    private slots:

        void            onInstallPlugin();

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

        void            requestHubModels();

    private:

        CUser                   m_currentUser;
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
        CHubPluginListView*     m_pHubView = nullptr;
        CHubPluginListView*     m_pWorkspaceView = nullptr;
        CHubPluginListView*     m_pLocalView = nullptr;
        CProcessDocWidget*      m_pDocWidget = nullptr;
        QPersistentModelIndex   m_currentModelIndex;
        ModelRequestStage       m_modelRequestStage = ModelRequestStage::IDLE;
        bool                    m_bAllModelsRequested = false;
        QElapsedTimer           m_lastUpdateTime;
        const int               m_updateFrequency = 3600000; //1 hour
};

#endif // CHUBDLG_H
