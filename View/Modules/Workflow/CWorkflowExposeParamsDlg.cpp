#include "CWorkflowExposeParamsDlg.h"

CWorkflowExposeParamsDlg::CWorkflowExposeParamsDlg(const WorkflowTaskPtr& taskPtr,
                                                   const WorkflowVertex& taskId,
                                                   const CWorkflow::ExposedParams& params,
                                                   QWidget *parent, Qt::WindowFlags f):
    CDialog(tr("Select exposed parameters"), parent, DEFAULT|EFFECT_ENABLED, f)
{
    m_taskId = taskId;
    m_workflowParams = params;
    setMinimumWidth(600);
    initLayout();
    initConnections();
    fillParamsTable(taskPtr);
}

CWorkflow::ExposedParams CWorkflowExposeParamsDlg::getExposedParams() const
{
    return m_taskParams;
}

void CWorkflowExposeParamsDlg::initLayout()
{
    // Parameters table
    m_pParamsTable = new QTableWidget(this);
    m_pParamsTable->setColumnCount(4);
    QStringList headerLabels = {tr("Exposed"), tr("Name"), tr("Exposed name"), tr("Description")};
    m_pParamsTable->setHorizontalHeaderLabels(headerLabels);
    m_pParamsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_pParamsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_pParamsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_pParamsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    // OK - Cancel buttons
    m_pOkBtn = new QPushButton(tr("OK"));
    m_pOkBtn->setDefault(true);
    m_pCancelBtn = new QPushButton(tr("Cancel"));

    QHBoxLayout* pBtnLayout = new QHBoxLayout;
    pBtnLayout->addWidget(m_pOkBtn);
    pBtnLayout->addWidget(m_pCancelBtn);

    // Main layout
    auto pMainLayout = getContentLayout();
    pMainLayout->addSpacing(5);
    pMainLayout->addWidget(m_pParamsTable);
    pMainLayout->addSpacing(5);
    pMainLayout->addLayout(pBtnLayout);
}

void CWorkflowExposeParamsDlg::initConnections()
{
    connect(m_pOkBtn, &QPushButton::clicked, this, &CWorkflowExposeParamsDlg::validate);
    connect(m_pCancelBtn, &QPushButton::clicked, this, &CWorkflowExposeParamsDlg::reject);
}

void CWorkflowExposeParamsDlg::fillParamsTable(const WorkflowTaskPtr &taskPtr)
{
    int i = 0;
    UMapString taskParams = taskPtr->getParamValues();
    m_pParamsTable->setRowCount(taskParams.size());

    for (auto it=taskParams.begin(); it!=taskParams.end(); ++it)
    {
        bool bExposed = false;
        QString paramName = QString::fromStdString(it->first);
        QString exposedName = paramName;
        QString description;

        auto ret = getExposedParam(it->first);
        if (ret.first)
        {
            bExposed = true;
            if (ret.second.getName().empty() == false)
                exposedName = QString::fromStdString(ret.second.getName());

            description = QString::fromStdString(ret.second.getDescription());
        }

        QCheckBox* pParamCheck = new QCheckBox;
        pParamCheck->setChecked(bExposed);
        QWidget* pContainerWidget = new QWidget;
        QHBoxLayout* pCheckLayout = new QHBoxLayout;
        pCheckLayout->setAlignment(Qt::AlignCenter);
        pCheckLayout->addWidget(pParamCheck);
        pContainerWidget->setLayout(pCheckLayout);
        m_pParamsTable->setCellWidget(i, 0, pContainerWidget);

        // Task param name: read-only
        QTableWidgetItem* pTaskParamName = new QTableWidgetItem(paramName);
        pTaskParamName->setFlags(pTaskParamName->flags() & !Qt::ItemIsEditable);
        m_pParamsTable->setItem(i, 1, pTaskParamName);

        // Workflow param name: editable
        QTableWidgetItem* pParamNameEdit = new QTableWidgetItem(exposedName);
        m_pParamsTable->setItem(i, 2, pParamNameEdit);

        // Workflow param description: editable
        QTableWidgetItem* pParamDescriptionEdit = new QTableWidgetItem(description);
        m_pParamsTable->setItem(i, 3, pParamDescriptionEdit);
        i++;
    }
}

std::pair<bool, CWorkflowParam> CWorkflowExposeParamsDlg::getExposedParam(const std::string& paramName)
{
    for (auto it=m_workflowParams.begin(); it!=m_workflowParams.end(); ++it)
    {
        WorkflowVertex paramTaskId = reinterpret_cast<WorkflowVertex>(it->second.getTaskId());
        if (paramTaskId == m_taskId && it->second.getTaskParamName() == paramName)
            return std::make_pair(true, it->second);
    }
    return std::make_pair(false, CWorkflowParam());
}

void CWorkflowExposeParamsDlg::validate()
{
    m_taskParams.clear();
    // Task ID
    std::uintptr_t id = reinterpret_cast<std::uintptr_t>(m_taskId);

    // Get parameters data
    for (int i=0; i<m_pParamsTable->rowCount(); ++i)
    {
        // Exposed status
        QWidget* pContainerWidget = m_pParamsTable->cellWidget(i, 0);
        QHBoxLayout* pLayout = static_cast<QHBoxLayout*>(pContainerWidget->layout());
        QCheckBox* pParamCheck = static_cast<QCheckBox*>(pLayout->itemAt(0)->widget());

        if (pParamCheck->isChecked())
        {
            // Task param name
            QTableWidgetItem* pItem = m_pParamsTable->item(i, 1);
            std::string taskParamName = pItem->text().toStdString();
            // Workflow param name
            pItem = m_pParamsTable->item(i, 2);
            std::string wfParamName = pItem->text().toStdString();
            // Description
            pItem = m_pParamsTable->item(i, 3);
            std::string description = pItem->text().toStdString();

            auto ret = m_taskParams.insert(std::make_pair(wfParamName, CWorkflowParam(wfParamName, description, id, taskParamName)));
            if (ret.second == false)
            {
                QMessageBox msgBox;
                QString msg = tr("Parameter name %1 is already set. Workflow parameter name must be unique.").arg(QString::fromStdString(wfParamName));
                msgBox.setText(msg);
                msgBox.exec();
                return;
            }
        }
    }

    // Check param name unicity
    for (auto it=m_taskParams.begin(); it!=m_taskParams.end(); ++it)
    {
        auto itFind = m_workflowParams.find(it->first);
        if (itFind != m_workflowParams.end() && itFind->second.getTaskId() != id)
        {
            QMessageBox msgBox;
            QString msg = tr("Parameter name %1 is already set. Workflow parameter name must be unique.").arg(QString::fromStdString(it->first));
            msgBox.setText(msg);
            msgBox.exec();
            return;
        }
    }

    emit accept();
}
