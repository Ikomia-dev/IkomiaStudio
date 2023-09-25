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

#include "CProcessPopupDlg.h"
#include "View/Process/CProcessDocWidget.h"

CProcessPopupDlg::CProcessPopupDlg(QWidget *parent, Qt::WindowFlags f):
    CDialog(tr("Algorithms"), parent, DEFAULT | MAXIMIZE_BUTTON, f)
{
    //Important note: style CDialog::EFFECT_ENABLES must not be used.
    //CHubDlg includes a QWebEngineView based on QOpenGLWidget,
    //and graphics effects are not supported for OpenGL-based widgets.
    initLayout();
    initConnections();

    auto screens = QGuiApplication::screens();
    assert(screens.size() > 0);
    QRect screenRect = screens[0]->availableGeometry();
    m_size = QSize(screenRect.width()*0.75, screenRect.height()*0.7);
    resize(m_size);
}

CProcessPopupDlg::~CProcessPopupDlg()
{
    CPyEnsureGIL gil;
    m_widgets.clear();
}

void CProcessPopupDlg::initLayout()
{
    setWindowOpacity(0.9);

    //Parameters widget -> popup
    m_pParamsWidget = new CProcessParameters(this);

    //Left part : TreeView
    auto pLeftWidget = createLeftWidget();

    //Right part : StackedWidget
    auto pRightWidget = createRightWidget();

    //Add left and right parts to Splitter
    QSplitter* pSplitter = new QSplitter;
    pSplitter->addWidget(pLeftWidget);
    pSplitter->addWidget(pRightWidget);
    pSplitter->setStretchFactor(0, 1);
    pSplitter->setStretchFactor(1, 3);

    auto pLayout = getContentLayout();
    pLayout->addWidget(pSplitter);
}

void CProcessPopupDlg::initConnections()
{
    connect(m_pTreeView, &QTreeView::clicked, this, &CProcessPopupDlg::onTreeViewClicked);
    connect(m_pTreeView, &QTreeView::clicked, m_pProcessView, &CProcessListView::onTreeViewClicked);

    connect(m_pSearchProcess, &QLineEdit::textChanged, this, &CProcessPopupDlg::onTextChanged);

    connect(m_pProcessView, &CProcessListView::doListViewDoubleCLicked, this, &CProcessPopupDlg::onListViewDoubleClicked);
    connect(m_pProcessView, &CProcessListView::doCurrentChanged, this, &CProcessPopupDlg::onCurrentChanged);
    connect(m_pProcessView, &CProcessListView::doAddProcess, this, &CProcessPopupDlg::onAddProcess);
    connect(m_pProcessView, &CProcessListView::doShowProcessInfo, this, &CProcessPopupDlg::onShowProcessInfo);
    connect(m_pProcessView, &CProcessListView::doListViewLicked, this, &CProcessPopupDlg::onListViewClicked);

    connect(m_pDocWidget, &CProcessDocWidget::doBack, [&]{ m_pRightStackedWidget->setCurrentIndex(0); });
    connect(m_pDocWidget, &CProcessDocWidget::doSave, [&](bool bFullEdit, const CTaskInfo& info)
    {
        emit doUpdateProcessInfo(bFullEdit, info);
    });
}

void CProcessPopupDlg::setCurrentUser(const CUser &user)
{
    if(m_pDocWidget)
        m_pDocWidget->setCurrentUser(user);
}

int CProcessPopupDlg::exec()
{
    // Update tree and listview with current search text
    emit doTextChanged(m_pSearchProcess->text());
    return QDialog::exec();
}

void CProcessPopupDlg::onSetProcessListModel(QSortFilterProxyModel* pModel)
{
    m_pProcessView->setModel(pModel);
    m_pProcessView->setModelColumn(1);
}

void CProcessPopupDlg::onSetProcessTreeModel(QSortFilterProxyModel* pModel)
{
    m_pTreeView->setModel(pModel);
    m_pTreeView->expandAll();
}

void CProcessPopupDlg::onSetWidgetInstance(const std::string &processName, WorkflowTaskWidgetPtr &widgetPtr)
{
    if(m_bQueryWidget == false)
        return;

    m_bQueryWidget = false;

    if(widgetPtr == nullptr)
        return;

    QModelIndex listItemIndex = m_pProcessView->getListView()->currentIndex();
    if(!listItemIndex.isValid())
        return;

    QString procName = QString::fromStdString(processName);
    m_widgets.insert(procName, widgetPtr);
    m_pParamsWidget->addWidget(widgetPtr.get(), procName);
    m_pParamsWidget->setProcessInfo(getProcessInfo(listItemIndex));
    m_pParamsWidget->setCurrentWidget(procName);
    adjustParamsWidget(listItemIndex);

    //Manage apply button signal
    auto pFunc = [this, processName](const WorkflowTaskParamPtr& pParam)
    {
        setResult(QDialog::Accepted);
        emit doAddProcess(processName, pParam);
        m_pParamsWidget->hide();
        done(1);
    };
    connect(widgetPtr.get(), &CWorkflowTaskWidget::doApplyProcess, pFunc);
}

void CProcessPopupDlg::onTreeViewClicked(const QModelIndex& index)
{
    m_pParamsWidget->hide();
    emit doTreeViewClicked(index);
}

void CProcessPopupDlg::onTextChanged(const QString& text)
{
    emit doTextChanged(text);
}

void CProcessPopupDlg::onActivateScrollBar()
{
    m_pTreeView->setHorizontalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
    m_pTreeView->setVerticalScrollBarPolicy ( Qt::ScrollBarAsNeeded );

    m_pProcessView->getListView()->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    m_pProcessView->getListView()->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
}

void CProcessPopupDlg::onListViewDoubleClicked(const QString& processName)
{
    emit doListViewDoubleClicked(processName);
}

void CProcessPopupDlg::onListViewClicked(const QModelIndex& index)
{
    auto indexList = m_pTreeView->model()->match(m_pTreeView->model()->index(0,0), Qt::DisplayRole, index.data(), 1, Qt::MatchRecursive);
    if(indexList.empty())
        return;

    m_pTreeView->setCurrentIndex(indexList.at(0));
}

void CProcessPopupDlg::onCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if(current != previous)
    {
        m_pParamsWidget->hide();

        if(m_pRightStackedWidget->currentIndex() == 1)
            onShowProcessInfo(current);
    }
}

void CProcessPopupDlg::onAddProcess(const QString& processName)
{
    auto itemIndex = m_pProcessView->getListView()->currentIndex();
    if(!itemIndex.isValid())
        return;

    auto it = m_widgets.find(processName);
    if(it == m_widgets.end())
    {
        m_bQueryWidget = true;
        emit doQueryWidgetInstance(processName.toStdString());
    }
    else
    {
        if(m_pParamsWidget->isCurrent(processName) && !m_pParamsWidget->isHidden())
            m_pParamsWidget->hide();
        else
        {
            m_pParamsWidget->setProcessInfo(getProcessInfo(itemIndex));
            m_pParamsWidget->setCurrentWidget(processName);
            adjustParamsWidget(itemIndex);
        }
    }
}

void CProcessPopupDlg::onShowProcessInfo(const QModelIndex& index)
{
    auto info = getProcessInfo(index);
    m_pDocWidget->setProcessInfo(info, false);
    m_pRightStackedWidget->setCurrentIndex(1);
}

void CProcessPopupDlg::done(int r)
{
    m_pParamsWidget->hide();
    QDialog::done(r);
}

void CProcessPopupDlg::reject()
{
    m_pParamsWidget->hide();
    return QDialog::reject();
}

QWidget *CProcessPopupDlg::createLeftWidget()
{
    m_pTreeView = new QTreeView;
    m_pTreeView->setObjectName("CTreeView");
    m_pTreeView->setHeaderHidden(true);
    return m_pTreeView;
}

QWidget *CProcessPopupDlg::createRightWidget()
{
    //1 - Search bar + ProcessListView
    m_pSearchProcess = new QLineEdit;
    m_pSearchProcess->setPlaceholderText(tr("<Search by keywords>"));

    QHBoxLayout* pHSearchLayout = new QHBoxLayout;
    pHSearchLayout->addStretch(1);
    pHSearchLayout->addWidget(m_pSearchProcess, 1);
    pHSearchLayout->addStretch(1);

    m_pProcessView = new CProcessListView;

    QVBoxLayout* pVProcessViewLayout = new QVBoxLayout;
    pVProcessViewLayout->setContentsMargins(0, 0, 0, 0);
    pVProcessViewLayout->addLayout(pHSearchLayout);
    pVProcessViewLayout->addWidget(m_pProcessView);

    QWidget* pProcessListViewWidget = new QWidget;
    pProcessListViewWidget->setLayout(pVProcessViewLayout);

    //2 - Documentation widget
    m_pDocWidget = new CProcessDocWidget(CProcessDocWidget::BACK);

    //Add to stacked widget
    m_pRightStackedWidget = new QStackedWidget;
    m_pRightStackedWidget->addWidget(pProcessListViewWidget);
    m_pRightStackedWidget->addWidget(m_pDocWidget);
    m_pRightStackedWidget->setCurrentIndex(0);
    return m_pRightStackedWidget;
}

CTaskInfo CProcessPopupDlg::getProcessInfo(const QModelIndex &index) const
{
    auto pProxyModel = static_cast<const CProcessTableProxyModel*>(index.model());
    auto srcIndex = pProxyModel->mapToSource(index);
    auto pModel = static_cast<const QSqlTableModel*>(srcIndex.model());
    auto record = pModel->record(index.row());

    CTaskInfo info;
    info.m_id = record.value("id").toInt();
    info.m_name = record.value("name").toString().toStdString();
    info.m_shortDescription = record.value("shortDescription").toString().toStdString();
    info.m_description = record.value("description").toString().toStdString();
    info.m_docLink = record.value("docLink").toString().toStdString();
    info.m_iconPath = record.value("iconPath").toString().toStdString();
    info.m_keywords = record.value("keywords").toString().toStdString();
    info.m_authors = record.value("authors").toString().toStdString();
    info.m_article = record.value("article").toString().toStdString();
    info.m_articleUrl = record.value("articleUrl").toString().toStdString();
    info.m_journal = record.value("journal").toString().toStdString();
    info.m_version = record.value("version").toString().toStdString();
    info.m_minIkomiaVersion = record.value("minIkomiaVersion").toString().toStdString();
    info.m_maxIkomiaVersion = record.value("maxIkomiaVersion").toString().toStdString();
    info.m_minPythonVersion = record.value("minPythonVersion").toString().toStdString();
    info.m_maxPythonVersion = record.value("minPythonVersion").toString().toStdString();
    info.m_license = record.value("license").toString().toStdString();
    info.m_repo = record.value("repository").toString().toStdString();
    info.m_originalRepo = record.value("originalRepository").toString().toStdString();
    info.m_createdDate = record.value("createdDate").toString().toStdString();
    info.m_modifiedDate = record.value("modifiedDate").toString().toStdString();
    info.m_year = record.value("year").toInt();
    info.m_language = record.value("language").toInt() == 0 ? ApiLanguage::CPP : ApiLanguage::PYTHON;
    info.m_bInternal = record.value("isInternal").toInt();
    info.m_os = static_cast<OSType>(record.value("os").toInt());
    info.m_algoType = static_cast<AlgoType>(record.value("algoType").toInt());
    info.m_algoTasks = record.value("algoTasks").toString().toStdString();
    return info;
}

void CProcessPopupDlg::adjustParamsWidget(const QModelIndex& index)
{
    m_pParamsWidget->show();
    m_pParamsWidget->fitToContent();

    QRect rcItem = m_pProcessView->getListView()->visualRect(index);
    QPoint itemPosition(rcItem.right(), (rcItem.top()+rcItem.bottom()) / 2);

    QRect screen = QApplication::desktop()->availableGeometry(this);
    auto size = m_pParamsWidget->sizeHint();
    auto right = itemPosition.x() - m_pParamsWidget->getBorderSize();
    auto top = itemPosition.y() - size.height()/2;
    auto globalTopRight = m_pProcessView->getListView()->mapToGlobal(QPoint(right, top));
    QSize pluginSize = m_pProcessView->getPluginSize();

    if(globalTopRight.y() + size.height() > screen.bottom())
        globalTopRight.setY(screen.bottom() - size.height());
    else if(globalTopRight.y() < 0)
        globalTopRight.setY(0);

    if(globalTopRight.x() + size.width() > screen.right())
        globalTopRight.setX(globalTopRight.x() - 0.1*pluginSize.width() - size.width()); // Because we put add icon on listItem at 0.9*width()

    m_pParamsWidget->move(globalTopRight);
}
