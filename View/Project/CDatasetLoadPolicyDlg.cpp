#include "CDatasetLoadPolicyDlg.h"

CDatasetLoadPolicyDlg::CDatasetLoadPolicyDlg(TreeItemType srcType, QWidget *parent, Qt::WindowFlags f)
    : CDialog(tr("Load image list"), parent, DEFAULT|EFFECT_ENABLED, f)
{
    m_srcType = srcType;
    m_relationship = Relationship::MANY_TO_ONE;
    m_extraDim = DataDimension::NONE;
    initLayout();
    initConnections();
}

DatasetLoadPolicy CDatasetLoadPolicyDlg::getLoadPolicy()
{
    return std::make_pair(m_relationship, m_extraDim);
}

void CDatasetLoadPolicyDlg::initLayout()
{
    initPolicyWidget();
    initDimensionWidget();

    QHBoxLayout* pBtnLayout = new QHBoxLayout;
    m_pOkBtn = new QPushButton(tr("OK"));
    m_pOkBtn->setDefault(true);
    pBtnLayout->addWidget(m_pOkBtn);
    m_pCancelBtn = new QPushButton(tr("Cancel"));
    pBtnLayout->addWidget(m_pCancelBtn);

    auto pLayout = getContentLayout();
    pLayout->addWidget(m_pPolicyWidget);
    pLayout->addWidget(m_pDimensionWidget);
    pLayout->addLayout(pBtnLayout);
    //Resize automatically when show/hide sub layouts
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    if(m_srcType == TreeItemType::DATASET)
        m_pPolicyWidget->hide();
    else
        m_pDimensionWidget->hide();
}

void CDatasetLoadPolicyDlg::initPolicyWidget()
{
    QLabel* pLabel = new QLabel(tr("Multiple files selected. Please choose the way you want to load the data:"));
    m_pRadioSingle = new QRadioButton(tr("Single dataset"));
    m_pRadioSingle->setChecked(true);
    m_pRadioStructured = new QRadioButton(tr("Structured dataset..."));
    m_pRadioMulti = new QRadioButton(tr("As many datasets as files (for complex file formats)"));

    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(pLabel);
    pLayout->addWidget(m_pRadioSingle);
    pLayout->addWidget(m_pRadioStructured);
    pLayout->addWidget(m_pRadioMulti);

    m_pPolicyWidget = new QWidget;
    m_pPolicyWidget->setLayout(pLayout);
}

void CDatasetLoadPolicyDlg::initDimensionWidget()
{
    m_pRadioList = new QRadioButton(tr("Simple list"));
    m_pRadioList->setChecked(true);
    m_pRadioVolume = new QRadioButton(tr("Volume"));
    m_pRadioPosition = new QRadioButton(tr("Position"));
    m_pRadioTime = new QRadioButton(tr("Time"));
    m_pRadioModality = new QRadioButton(tr("Modality"));

    QVBoxLayout* pDimsLayout = new QVBoxLayout;
    pDimsLayout->addWidget(m_pRadioList);
    pDimsLayout->addWidget(m_pRadioVolume);
    pDimsLayout->addWidget(m_pRadioTime);
    pDimsLayout->addWidget(m_pRadioPosition);
    pDimsLayout->addWidget(m_pRadioModality);

    auto pDimGroupBox = new QGroupBox(tr("Extra dimension choice:"));
    pDimGroupBox->setLayout(pDimsLayout);

    auto pLayout = new QVBoxLayout;
    pLayout->addWidget(pDimGroupBox);

    m_pDimensionWidget = new QWidget;
    m_pDimensionWidget->setLayout(pLayout);
}

void CDatasetLoadPolicyDlg::initConnections()
{
    //Load policy
    connect(m_pRadioMulti, &QRadioButton::toggled, this, &CDatasetLoadPolicyDlg::onToggledMulti);
    connect(m_pRadioSingle, &QRadioButton::toggled, this, &CDatasetLoadPolicyDlg::onToggledSingle);
    connect(m_pRadioStructured, &QRadioButton::toggled, this, &CDatasetLoadPolicyDlg::onToggledStructured);
    //Dimensions
    connect(m_pRadioList, &QRadioButton::toggled, this, &CDatasetLoadPolicyDlg::onToggledSimpleList);
    connect(m_pRadioVolume, &QRadioButton::toggled, this, &CDatasetLoadPolicyDlg::onToggledVolume);
    connect(m_pRadioTime, &QRadioButton::toggled, this, &CDatasetLoadPolicyDlg::onToggledTime);
    connect(m_pRadioPosition, &QRadioButton::toggled, this, &CDatasetLoadPolicyDlg::onToggledPosition);
    connect(m_pRadioModality, &QRadioButton::toggled, this, &CDatasetLoadPolicyDlg::onToggledModality);
    //OK - Cancel
    connect(m_pOkBtn, &QPushButton::clicked, this, &CDatasetLoadPolicyDlg::accept);
    connect(m_pCancelBtn, &QPushButton::clicked, this, &CDatasetLoadPolicyDlg::reject);
}

void CDatasetLoadPolicyDlg::onToggledMulti(bool bChecked)
{
    if(bChecked)
    {
        if(!m_pDimensionWidget->isHidden())
            m_pDimensionWidget->hide();

        m_relationship = Relationship::ONE_TO_ONE;
        m_extraDim = DataDimension::NONE;
    }
}

void CDatasetLoadPolicyDlg::onToggledSingle(bool bChecked)
{
    if(bChecked)
    {
        if(!m_pDimensionWidget->isHidden())
            m_pDimensionWidget->hide();

        m_relationship = Relationship::MANY_TO_ONE;
        m_extraDim = DataDimension::NONE;
    }
}

void CDatasetLoadPolicyDlg::onToggledStructured(bool bChecked)
{
    if(bChecked)
    {
        m_pDimensionWidget->show();
        m_relationship = Relationship::MANY_TO_ONE;

        if(m_pRadioList->isChecked())
            m_extraDim = DataDimension::NONE;
        else if(m_pRadioVolume->isChecked())
            m_extraDim = DataDimension::VOLUME;
        else if(m_pRadioTime->isChecked())
            m_extraDim = DataDimension::TIME;
        else if(m_pRadioPosition->isChecked())
            m_extraDim = DataDimension::POSITION;
        else if(m_pRadioModality->isChecked())
            m_extraDim = DataDimension::MODALITY;
    }
}

void CDatasetLoadPolicyDlg::onToggledSimpleList(bool bChecked)
{
    if(bChecked)
        m_extraDim = DataDimension::NONE;
}

void CDatasetLoadPolicyDlg::onToggledVolume(bool bChecked)
{
    if(bChecked)
        m_extraDim = DataDimension::VOLUME;
}

void CDatasetLoadPolicyDlg::onToggledPosition(bool bChecked)
{
    if(bChecked)
        m_extraDim = DataDimension::POSITION;
}

void CDatasetLoadPolicyDlg::onToggledTime(bool bChecked)
{
    if(bChecked)
        m_extraDim = DataDimension::TIME;
}

void CDatasetLoadPolicyDlg::onToggledModality(bool bChecked)
{
    if(bChecked)
        m_extraDim = DataDimension::MODALITY;
}
