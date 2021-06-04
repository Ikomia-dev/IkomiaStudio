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

#ifndef CPROCESSPANE_H
#define CPROCESSPANE_H

#include "../Common/CPane.h"
#include "Model/CMainModel.h"
#include "CProcessParameters.h"
#include "CProcessListPopup.h"

class CProcessPane : public QWidget
{
    Q_OBJECT

    public:

        CProcessPane(QWidget* parent = nullptr);
        ~CProcessPane();

        void                setCurrentUser(const CUser& user);
        void                update();

        QTreeView*          getTreeView();
        CProcessListPopup*  getListPopup();
        CProcessParameters* getProcessParameters();
        QLineEdit*          getSearchBox();

    public slots:

        void    onSetModel(QSortFilterProxyModel* pModel);
        void    onSetTableModel(QSortFilterProxyModel* pTableModel);
        void    onTreeItemClicked(const QModelIndex& index);
        void    onSearchProcess(const QString& text);
        void    onSetWidgetInstance(const std::string &processName, WorkflowTaskWidgetPtr& widgetPtr);
        void    onSetProcessInfo(const CTaskInfo& info);
        void    onShowWidgetFromList(const QString& name);
        void    onAllProcessReloaded();
        void    onProcessReloaded(const QString& name);

    signals:

        void    doApplyProcess(const std::string& name, const std::shared_ptr<CWorkflowTaskParam>& pParam);
        void    doSearchProcess(const QString& text);
        void    doInitWidgetFactoryConnection();
        void    doEndWidgetFactoryConnection();
        void    doQueryWidgetInstance(const std::string& processName);
        void    doQueryProcessInfo(const std::string& processName);
        void    doTreeViewClicked(const QModelIndex& index);

    protected:

        void    hideEvent(QHideEvent* event) override;
        void    resizeEvent(QResizeEvent *event) override;

    private:

        void    initLayout();
        void    initConnections();

        void    expandProcessItem(const QModelIndex& index, QSortFilterProxyModel* pModel, int depth);
        void    expandProcess(QSortFilterProxyModel* pModel, int depth);

        void    showProcessWidget(const std::string& processName);
        void    showProcessWidgetFromList(const std::string& processName);
        void    showProcessListWidget(const std::string& categoryName);

        void    adjustProcessWidget();

        void    updateWidgetPosition(QWidget* pWidget, QPoint itemPosition, int borderSize) ;

    private:

        using UMapNameWidget = std::unordered_map<std::string, WorkflowTaskWidgetPtr>;

        QTreeView*          m_pProcessTree = nullptr;
        //We need to have persistent storage due to shared_ptr
        //-> unique_ptr would have been best but Boost.Python is not compatible
        UMapNameWidget      m_widgets;
        CProcessParameters* m_pParamsWidget = nullptr;
        CProcessListPopup*  m_pProcessList = nullptr;
        QLineEdit*          m_pSearchBox = nullptr;
        QAbstractItemView*  m_pCurrentView = nullptr;
        bool                m_bQueryWidget = false;
};

#endif // CPROCESSPANE_H
