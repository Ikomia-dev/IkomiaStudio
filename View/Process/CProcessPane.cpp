// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "CProcessPane.h"
#include <QSplitter>
#include <QLineEdit>

CProcessPane::CProcessPane(QWidget *parent) : QWidget(parent)
{
    initLayout();
    initConnections();
}

CProcessPane::~CProcessPane()
{
    CPyEnsureGIL gil;
    m_widgets.clear();
}

void CProcessPane::setCurrentUser(const CUser &user)
{
    if(m_pParamsWidget)
        m_pParamsWidget->setCurrentUser(user);
}

void CProcessPane::update()
{
    // Update tree and listview process with current search text
    emit doSearchProcess(m_pSearchBox->text());
}

QTreeView* CProcessPane::getTreeView()
{
    return m_pProcessTree;
}

CProcessListPopup* CProcessPane::getListPopup()
{
    return m_pProcessList;
}

CProcessParameters* CProcessPane::getProcessParameters()
{
    return m_pParamsWidget;
}

QLineEdit* CProcessPane::getSearchBox()
{
    return m_pSearchBox;
}

void CProcessPane::onSetModel(QSortFilterProxyModel* pModel)
{
    assert(pModel);
    m_pProcessTree->setModel(pModel);

    // Expand all if user performs search otherwise expand to opencv modules
    if(m_pSearchBox->text().isEmpty())
        expandProcess(pModel, 4);
    else
        m_pProcessTree->expandAll();
}

void CProcessPane::onSetTableModel(QSortFilterProxyModel* pTableModel)
{
    assert(pTableModel);
    m_pProcessList->setModel(pTableModel);
}

void CProcessPane::onTreeItemClicked(const QModelIndex &index)
{
    assert(index.isValid());
    // Don't forget that we are using a proxy model
    auto pProxyModel = static_cast<CProcessProxyModel*>(m_pProcessTree->model());
    auto currIndex = pProxyModel->mapToSource(index);
    auto* pItem = static_cast<ProcessTreeItem*>(currIndex.internalPointer());
    assert(pItem);

    // Set current as TreeView by default because parameters or processlist popup nearby
    m_pCurrentView = m_pProcessTree;

    if(pItem->getTypeId() == TreeItemType::PROCESS)
    {
        m_pProcessList->hide();
        showProcessWidget(pItem->getName());
    }
    else if(pItem->getTypeId() == TreeItemType::FOLDER)
    {
        m_pParamsWidget->hide();
        showProcessListWidget(pItem->getName());
    }
    else
    {
        m_pProcessList->hide();
        m_pParamsWidget->hide();
    }
}

void CProcessPane::onSearchProcess(const QString& text)
{
    m_pParamsWidget->hide();
    m_pProcessList->hide();
    emit doSearchProcess(text);
}

void CProcessPane::onSetWidgetInstance(const std::string& processName, ProtocolTaskWidgetPtr &widgetPtr)
{
    if(m_bQueryWidget == false)
        return;

    m_bQueryWidget = false;

    try
    {
        emit doEndWidgetFactoryConnection();
        if(widgetPtr)
        {
            m_widgets.insert(std::make_pair(processName, widgetPtr));
            m_pParamsWidget->addWidget(widgetPtr.get(), QString::fromStdString(processName));
            emit doQueryProcessInfo(processName);
            m_pParamsWidget->setCurrentWidget(QString::fromStdString(processName));
            adjustProcessWidget();

            //Manage apply button signal
            auto pFunc = [this](const ProtocolTaskParamPtr& pParam)
            {
                if(m_pCurrentView == m_pProcessTree)
                {
                    auto pProxyModel = static_cast<CProcessProxyModel*>(m_pCurrentView->model());
                    auto pItem = static_cast<ProcessTreeItem*>(pProxyModel->mapToSource(m_pCurrentView->currentIndex()).internalPointer());
                    emit doApplyProcess(pItem->getName(), pParam);
                }
                else
                {
                    auto pProxyModel = static_cast<QSortFilterProxyModel*>(m_pCurrentView->model());
                    auto currIndex = pProxyModel->mapToSource(m_pCurrentView->currentIndex());
                    // Get QSqlTableModel
                    auto pTable = static_cast<const QSqlTableModel*>(currIndex.model());
                    // Get name from table process (id, name, description, folder_id)
                    auto name = pTable->record(m_pCurrentView->currentIndex().row()).value(1).toString();
                    emit doApplyProcess(name.toStdString(), pParam);
                }
                m_pParamsWidget->hide();
                m_pProcessList->hide();
            };
            connect(widgetPtr.get(), &CProtocolTaskWidget::doApplyProcess, pFunc);
        }
    }
    catch(std::exception& e)
    {
        qCritical().noquote() << QString::fromStdString(e.what());
    }
}

void CProcessPane::onSetProcessInfo(const CProcessInfo &info)
{
    m_pParamsWidget->setProcessInfo(info);
}

void CProcessPane::onShowWidgetFromList(const QString& name)
{
    showProcessWidgetFromList(name.toStdString());
}

void CProcessPane::onAllProcessReloaded()
{
    CPyEnsureGIL gil;
    m_pParamsWidget->hide();
    m_widgets.clear();
    m_pParamsWidget->clear();
}

void CProcessPane::onProcessReloaded(const QString &name)
{
    CPyEnsureGIL gil;
    m_pParamsWidget->hide();
    m_widgets.erase(name.toStdString());
    m_pParamsWidget->remove(name);
}

void CProcessPane::showProcessWidget(const std::string &processName)
{   
    QModelIndex itemIndex = m_pProcessTree->currentIndex();
    if(itemIndex.isValid() == false)
        return;

    auto it = m_widgets.find(processName);
    if(it == m_widgets.end())
    {
        m_bQueryWidget = true;
        emit doQueryWidgetInstance(processName);
    }
    else
    {
        if(m_pParamsWidget->isCurrent(QString::fromStdString(processName)) && !m_pParamsWidget->isHidden())
            m_pParamsWidget->hide();
        else
        {
            emit doQueryProcessInfo(processName);
            m_pParamsWidget->setCurrentWidget(QString::fromStdString(processName));
            adjustProcessWidget();
        }
    }
}

void CProcessPane::showProcessWidgetFromList(const std::string& processName)
{
    QModelIndex itemIndex = m_pProcessList->getListView()->currentIndex();
    if(itemIndex.isValid() == false)
        return;

    auto it = m_widgets.find(processName);
    if(it == m_widgets.end())
    {
        m_bQueryWidget = true;
        emit doQueryWidgetInstance(processName);
    }
    else
    {
        if(m_pParamsWidget->isCurrent(QString::fromStdString(processName)) && !m_pParamsWidget->isHidden())
            m_pParamsWidget->hide();
        else
        {
            emit doQueryProcessInfo(processName);
            m_pParamsWidget->setCurrentWidget(QString::fromStdString(processName));
            adjustProcessWidget();
        }
    }
}

void CProcessPane::showProcessListWidget(const std::string& categoryName)
{
    QModelIndex itemIndex = m_pProcessTree->currentIndex();
    if(itemIndex.isValid() == false)
        return;

    if(m_pProcessList->isCurrent(QString::fromStdString(categoryName)) && !m_pProcessList->isHidden())
        m_pProcessList->hide();
    else
    {
        emit doTreeViewClicked(itemIndex);
        // Use Tool flag to ensure on top position when app is active
        // Must be set before every show() (for OSX)
        m_pProcessList->setWindowFlags(m_pProcessList->windowFlags() | Qt::Tool);
        m_pProcessList->show();
        m_pProcessList->setCurrentCategory(QString::fromStdString(categoryName));
        m_pProcessList->resizeListView();
        //Update position
        QRect rcItem = m_pProcessTree->visualRect(itemIndex);
        QPoint position((rcItem.left()+rcItem.right()) / 2, (rcItem.top()+rcItem.bottom()) / 2);
        updateWidgetPosition(m_pProcessList, position, m_pProcessList->getBorderSize());
        // Set current view as process list view in order to display parameters nearby
        m_pCurrentView = m_pProcessList->getListView();
    }
}

void CProcessPane::adjustProcessWidget()
{
    //Show widget and adjust size
    // Use Tool flag to ensure on top position when app is active
    // Must be set before every show()
    m_pParamsWidget->setWindowFlags(m_pParamsWidget->windowFlags() | Qt::Tool);
    m_pParamsWidget->show();
    m_pParamsWidget->fitToContent();

    //Set position
    QModelIndex treeItemIndex = m_pCurrentView->currentIndex();
    QRect rcItem = m_pCurrentView->visualRect(treeItemIndex);
    QPoint position((rcItem.left()+rcItem.right()) / 2, (rcItem.top()+rcItem.bottom()) / 2);
    updateWidgetPosition(m_pParamsWidget, position, m_pParamsWidget->getBorderSize());
}

void CProcessPane::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    m_pProcessList->hide();
    m_pParamsWidget->hide();
}

void CProcessPane::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    auto topRight = m_pProcessTree->frameGeometry().topRight();
    auto globalTopRight = m_pProcessTree->mapToGlobal(topRight);
    m_pParamsWidget->move(globalTopRight);
    m_pProcessList->move(globalTopRight);
}

void CProcessPane::initLayout()
{
    // Search Box
    m_pSearchBox = new QLineEdit;
    m_pSearchBox->setPlaceholderText(tr("<Search by keywords>"));

    //Process tree
    m_pProcessTree = new QTreeView;
    m_pProcessTree->setObjectName("CTreeView");
    m_pProcessTree->setHeaderHidden(true);
    m_pProcessTree->setItemsExpandable(true);
    m_pProcessTree->setExpandsOnDoubleClick(true);

    // Process Tree + search box
    QWidget* pSearchTree = new QWidget;
    QVBoxLayout* pSearchTreeLayout = new QVBoxLayout;
    pSearchTreeLayout->addWidget(m_pSearchBox);
    pSearchTreeLayout->addWidget(m_pProcessTree);
    pSearchTree->setLayout(pSearchTreeLayout);

    QToolBox* pProcessTreeToolBox = new QToolBox;
    pProcessTreeToolBox->addItem(pSearchTree, tr("Process library"));

    //Process params
    // Parent 'this' is important here because it ensures QDialog to be on top of parent (linux/win)
    m_pParamsWidget = new CProcessParameters(this);
    m_pParamsWidget->hide();    

    // Process list popup
    // Parent 'this' is important here because it ensures QDialog to be on top of parent (linux/win)
    m_pProcessList = new CProcessListPopup(this);
    m_pProcessList->hide();

    //Add to layout
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(pProcessTreeToolBox);
    setLayout(pLayout);
}

void CProcessPane::initConnections()
{
    connect(m_pProcessTree, &QTreeView::clicked, this, &CProcessPane::onTreeItemClicked);
    connect(m_pSearchBox, &QLineEdit::textChanged, this, &CProcessPane::onSearchProcess);
    connect(m_pProcessList, &CProcessListPopup::doShowWidget, this, &CProcessPane::onShowWidgetFromList);
}

void CProcessPane::expandProcessItem(const QModelIndex& index, QSortFilterProxyModel* pModel, int depth)
{
    assert(pModel);

    if(depth == 0)
        return;

    depth--;

    // Check is there are children
    if(!pModel->hasChildren(index))
        return;

   // Get first child
   auto indFirst = pModel->index(0, 0, index);
   // Map index from proxy model to source model in order to retrieve pointer
   auto srcIndex = pModel->mapToSource(indFirst);
   // Get item pointer
   auto pItem = static_cast<ProcessTreeItem*>(srcIndex.internalPointer());
   // Check if first child is folder
   bool bIsFolder = true;
    if(pItem != nullptr)
        bIsFolder = pItem->getTypeId() == TreeItemType::FOLDER;

    // Expand only folders which contain at least one folder
    if(index.isValid() && bIsFolder)
        m_pProcessTree->expand(index);

    // Iterate recursively
    int row = pModel->rowCount(index);
    for(int i=0; i<row; ++i)
        expandProcessItem(pModel->index(i, 0, index), pModel, depth);
}

void CProcessPane::expandProcess(QSortFilterProxyModel* pModel, int depth)
{
    auto rootInd = m_pProcessTree->rootIndex();

    expandProcessItem(rootInd, pModel, depth);
}

void CProcessPane::updateWidgetPosition(QWidget* pWidget, QPoint itemPosition, int borderSize)
{
    QRect screen = QApplication::desktop()->availableGeometry(this);
    auto size = pWidget->size();
    auto right = m_pCurrentView->frameGeometry().right() - borderSize;
    auto top = itemPosition.y() - size.height()/2;
    auto globalTopRight = m_pCurrentView->mapToGlobal(QPoint(right, top));

    if(globalTopRight.y() + size.height() > screen.bottom())
        globalTopRight.setY(screen.bottom() - size.height());
    else if(globalTopRight.y() < 0)
        globalTopRight.setY(0);

    pWidget->move(globalTopRight);
}
