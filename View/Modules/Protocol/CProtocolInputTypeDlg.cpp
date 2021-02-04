#include "CProtocolInputTypeDlg.h"
#include "Model/Project/CProjectDataProxyModel.h"

CProtocolInputTypeDlg::CProtocolInputTypeDlg(QWidget *parent, Qt::WindowFlags f)
    : CDialog(tr("Select source of input data"), parent, DEFAULT|EFFECT_ENABLED, f)
{
    resize(QSize(300, 350));
    initLayout();
    initConnections();
}

void CProtocolInputTypeDlg::setModel(CProjectDataProxyModel *pModel)
{
    m_pModel = pModel;
    m_pTreeView->setModel(pModel);
    m_pTreeView->expandAll();
}

CProtocolInput CProtocolInputTypeDlg::getInput() const
{
    return m_input;
}

int CProtocolInputTypeDlg::exec()
{
    return QDialog::exec();
}

void CProtocolInputTypeDlg::onInputTypeSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    TreeItemType inputType = static_cast<TreeItemType>(current->data(Qt::UserRole).toInt());
    m_input.setType(inputType);
    m_input.setMode(ProtocolInputMode::FIXED_DATA);

    switch(inputType)
    {
        case TreeItemType::IMAGE:
        case TreeItemType::VIDEO:
        case TreeItemType::FOLDER:
            showProjectView();
            break;
        case TreeItemType::DATASET:
            showInputOptions();
            break;
        default: break;
    }
}

void CProtocolInputTypeDlg::onBack()
{
    int index = m_pStackedWidget->currentIndex();
    switch(index)
    {
        case 0:
            break;
        case 1:
            showInputTypes();
            break;
        case 2:
            if(m_input.getType() == TreeItemType::DATASET)
                showInputOptions();
            else
                showInputTypes();
            break;
    }
}

void CProtocolInputTypeDlg::onValidate()
{
    int pageIndex = m_pStackedWidget->currentIndex();
    if(pageIndex == 1)
    {
        validateFilters();
        showProjectView();
    }
    else
        validateProjectItems();
}

void CProtocolInputTypeDlg::initLayout()
{
    initInputTypesWidget();
    initInputOptionWidget();
    initProjectViewWidget();

    m_pStackedWidget = new QStackedWidget;
    m_pStackedWidget->addWidget(m_pInputTypesWidget);
    m_pStackedWidget->addWidget(m_pInputOptionWidget);
    m_pStackedWidget->addWidget(m_pProjectViewWidget);

    m_pBackBtn = new QPushButton(QIcon(":/Images/back.png"), "");
    m_pBackBtn->setToolTip(tr("Back"));
    m_pBackBtn->setVisible(false);
    connect(m_pBackBtn, &QPushButton::clicked, this, &CProtocolInputTypeDlg::onBack);

    m_pNextBtn = new QPushButton(tr("Next"));
    m_pNextBtn->setVisible(false);
    connect(m_pNextBtn, &QPushButton::clicked, this, &CProtocolInputTypeDlg::onValidate);

    m_pCancelBtn = new QPushButton(tr("Cancel"));
    connect(m_pCancelBtn, &QPushButton::clicked, this, &CProtocolInputTypeDlg::reject);

    QHBoxLayout* pBtnLayout = new QHBoxLayout;
    pBtnLayout->addWidget(m_pBackBtn);
    pBtnLayout->addWidget(m_pNextBtn);
    pBtnLayout->addWidget(m_pCancelBtn);

    auto* pLayout = getContentLayout();
    pLayout->addWidget(m_pStackedWidget);
    pLayout->addLayout(pBtnLayout);
}

void CProtocolInputTypeDlg::initInputTypesWidget()
{
    m_pInputTypesWidget = new QListWidget;
    m_pInputTypesWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pInputTypesWidget->setViewMode(QListWidget::IconMode);
    m_pInputTypesWidget->setFlow(QListWidget::LeftToRight);
    m_pInputTypesWidget->setResizeMode(QListWidget::Adjust);
    m_pInputTypesWidget->setGridSize(QSize(80,80));
    m_pInputTypesWidget->setSpacing(10);

    auto pItemImage = new QListWidgetItem(QIcon(":/Images/images.png"), tr("Image(s)"), m_pInputTypesWidget);
    pItemImage->setData(Qt::UserRole, static_cast<int>(TreeItemType::IMAGE));

    auto pItemVideo = new QListWidgetItem(QIcon(":/Images/video.png"), tr("Video(s)"), m_pInputTypesWidget);
    pItemVideo->setData(Qt::UserRole, static_cast<int>(TreeItemType::VIDEO));

    auto pItemDataset = new QListWidgetItem(QIcon(":/Images/dataset.png"), tr("Dataset(s)"), m_pInputTypesWidget);
    pItemDataset->setData(Qt::UserRole, static_cast<int>(TreeItemType::DATASET));

    auto pItemFolder = new QListWidgetItem(QIcon(":/Images/folder.png"), tr("Folder(s)"), m_pInputTypesWidget);
    pItemFolder->setData(Qt::UserRole, static_cast<int>(TreeItemType::FOLDER));
}

void CProtocolInputTypeDlg::initInputOptionWidget()
{
    auto pLabelInputOption = new QLabel(tr("Select the type of data you want to process"));
    m_pCheckSimpleDataset = new QCheckBox(tr("Simple dataset"));
    m_pCheckSimpleDataset->setChecked(true);
    m_pCheckVolumeDataset = new QCheckBox(tr("Volume dataset"));
    m_pCheckVolumeDataset->setChecked(true);
    m_pCheckTimeDataset = new QCheckBox(tr("Time dataset"));
    m_pCheckTimeDataset->setChecked(true);

    auto pVLayout = new QVBoxLayout;
    pVLayout->addWidget(pLabelInputOption);
    pVLayout->addWidget(m_pCheckSimpleDataset);
    pVLayout->addWidget(m_pCheckVolumeDataset);
    pVLayout->addWidget(m_pCheckTimeDataset);
    pVLayout->addStretch();

    m_pInputOptionWidget = new QWidget;
    m_pInputOptionWidget->setLayout(pVLayout);
}

void CProtocolInputTypeDlg::initProjectViewWidget()
{
    m_pTreeView = new QTreeView;
    m_pTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_pTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pTreeView->setHeaderHidden(true);

    QVBoxLayout* pWidgetLayout = new QVBoxLayout;
    pWidgetLayout->addWidget(m_pTreeView);

    m_pProjectViewWidget = new QWidget;
    m_pProjectViewWidget->setLayout(pWidgetLayout);
}

void CProtocolInputTypeDlg::initConnections()
{
    connect(m_pInputTypesWidget, &QListWidget::currentItemChanged, this, &CProtocolInputTypeDlg::onInputTypeSelected);

    connect(m_pTreeView, &QTreeView::doubleClicked, this, &CProtocolInputTypeDlg::onValidate);
}

void CProtocolInputTypeDlg::showInputTypes()
{
    m_pBackBtn->setVisible(false);
    m_pNextBtn->setVisible(true);
    m_pStackedWidget->setCurrentIndex(0);
}

void CProtocolInputTypeDlg::showInputOptions()
{
    m_pBackBtn->setVisible(true);
    m_pNextBtn->setVisible(true);
    m_pStackedWidget->setCurrentIndex(1);
}

void CProtocolInputTypeDlg::showProjectView()
{
    emit doQueryProjectDataProxyModel(m_input.getType(), m_input.getDataFilters());
    m_pBackBtn->setVisible(true);
    m_pNextBtn->setText("OK");
    m_pNextBtn->setVisible(true);
    m_pStackedWidget->setCurrentIndex(2);
}

void CProtocolInputTypeDlg::validateFilters()
{
    if(m_pCheckSimpleDataset->isChecked())
        m_input.addDataFilters(DataDimension::IMAGE);

    if(m_pCheckVolumeDataset->isChecked())
        m_input.addDataFilters(DataDimension::VOLUME);

    if(m_pCheckTimeDataset->isChecked())
        m_input.addDataFilters(DataDimension::TIME);
}

void CProtocolInputTypeDlg::validateProjectItems()
{
    if(m_pModel == nullptr)
        QDialog::reject();
    else
    {
        auto selectedItems = m_pTreeView->selectionModel()->selectedIndexes();
        for(int i=0; i<selectedItems.size(); ++i)
            m_input.appendModelIndex(m_pModel->mapToSource(selectedItems[i]));

        QDialog::accept();
    }
}
