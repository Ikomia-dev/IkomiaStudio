#include "CWorkflowPublishDlg.h"
#include "Main/AppTools.hpp"

CWorkflowPublishDlg::CWorkflowPublishDlg(const QString& wfName, const QString& wfDescription,
                                         const QJsonArray &projects,
                                         const CUser &user,
                                         QWidget *parent, Qt::WindowFlags f)
    : CDialog(tr("Workflow publication"), parent, DEFAULT|EFFECT_ENABLED, f)
{
    m_workflowName = wfName;
    m_workflowDescription = wfDescription;
    m_projects = projects;
    initLayout(user);
    initConnections();
}

QString CWorkflowPublishDlg::getWorkflowName() const
{
    return m_workflowName;
}

QString CWorkflowPublishDlg::getWorkflowDescription() const
{
    return m_workflowDescription;
}

QString CWorkflowPublishDlg::getProjectName() const
{
    if (m_pRadioNewProject->isChecked())
        return m_pNewProjectName->text();
    else
    {
        QJsonObject project = getProjectFromPath(m_pComboProject->currentData().toString());
        return project["name"].toString();
    }
}

QString CWorkflowPublishDlg::getProjectDescription() const
{
    if (m_pRadioNewProject->isChecked())
        return m_pNewProjectDescription->toPlainText();
    else
    {
        QJsonObject project = getProjectFromPath(m_pComboProject->currentData().toString());
        return project["description"].toString();
    }
}

QString CWorkflowPublishDlg::getNamespacePath() const
{
    if (m_pRadioNewProject->isChecked())
        return m_pComboNamespace->currentData().toString();
    else
        return m_pComboProject->currentData().toString();
}

bool CWorkflowPublishDlg::isNewProject() const
{
    return m_pRadioNewProject->isChecked();
}

void CWorkflowPublishDlg::initLayout(const CUser &user)
{
    const int minPlainTextEditHeight = 75;
    const int minCol1Width = 120;
    const int minCol2Width = 250;

    // Workflow name
    auto pLabelWfName = new QLabel(tr("Workflow name"));
    m_pEditWfName = new QLineEdit(m_workflowName);

    // Workflow desciption
    auto pLabelWfDesc = new QLabel(tr("Workflow description"));
    m_pEditDescription = new QPlainTextEdit(m_workflowDescription);

    // Project section
    auto pLabelProject = new QLabel(tr("Project"));
    m_pRadioExistingProject = new QRadioButton(tr("Add to existing project"));
    m_pRadioNewProject = new QRadioButton(tr("Create new project"));
    m_pRadioExistingProject->setChecked(true);

    // Existing project subsection
    auto pLabelExistingProjectName = new QLabel(tr("Project name"));
    pLabelExistingProjectName->setMinimumWidth(minCol1Width);
    m_pComboProject = new QComboBox();
    m_pComboProject->setMinimumWidth(minCol2Width);

    for (int i=0; i<m_projects.size(); ++i)
    {
        QJsonObject project = m_projects[i].toObject();
        auto displayName = getProjectDisplayName(project);
        m_pComboProject->addItem(displayName, project["path"].toString());
    }

    auto pExistingProjectLayout = new QGridLayout();
    pExistingProjectLayout->addWidget(pLabelExistingProjectName, 0, 0);
    pExistingProjectLayout->addWidget(m_pComboProject, 0, 1);

    auto pExistingProjectWidget = new QWidget();
    pExistingProjectWidget->setLayout(pExistingProjectLayout);

    // New project subsection
    auto pLabelNewProjectName = new QLabel(tr("Project name"));
    pLabelNewProjectName->setMinimumWidth(minCol1Width);
    m_pNewProjectName = new QLineEdit("new-project");
    auto pLabelNewProjectDesc = new QLabel(tr("Project description"));
    m_pNewProjectDescription = new QPlainTextEdit();
    m_pNewProjectDescription->setFixedHeight(minPlainTextEditHeight);
    auto pLabelWorkspace = new QLabel(tr("Parent workspace"));
    m_pComboNamespace = new QComboBox();
    m_pComboNamespace->setMinimumWidth(minCol2Width);

    for (size_t i=0; i<user.getNamespaceCount(); ++i)
    {
        auto ns = user.getNamespace(i);
        m_pComboNamespace->addItem(Utils::User::getNamespaceDisplayName(ns.m_path), ns.m_path);
    }

    auto pNewProjectLayout = new QGridLayout();
    pNewProjectLayout->addWidget(pLabelNewProjectName, 0, 0);
    pNewProjectLayout->addWidget(m_pNewProjectName, 0, 1);
    pNewProjectLayout->addWidget(pLabelNewProjectDesc, 1, 0);
    pNewProjectLayout->addWidget(m_pNewProjectDescription, 1, 1);
    pNewProjectLayout->addWidget(pLabelWorkspace, 2, 0);
    pNewProjectLayout->addWidget(m_pComboNamespace, 2, 1);

    auto pNewProjectWidget = new QWidget();
    pNewProjectWidget->setLayout(pNewProjectLayout);

    // Stacked widget
    m_pProjectStackWidget = new CResizeStackedWidget();
    m_pProjectStackWidget->addWidget(pExistingProjectWidget);
    m_pProjectStackWidget->addWidget(pNewProjectWidget);
    m_pProjectStackWidget->setCurrentIndex(0);

    auto pLayout = new QGridLayout();
    pLayout->addWidget(pLabelWfName, 0, 0);
    pLayout->addWidget(m_pEditWfName, 0, 1, 1, 2);
    pLayout->addWidget(pLabelWfDesc, 1, 0);
    pLayout->addWidget(m_pEditDescription, 1, 1, 1, 2);
    pLayout->addWidget(pLabelProject, 2, 0, 1, 3);
    pLayout->addWidget(m_pRadioExistingProject, 3, 1, 1, 2);
    pLayout->addWidget(m_pRadioNewProject, 4, 1, 1, 2);
    pLayout->addWidget(m_pProjectStackWidget, 5, 1, 1, 2);

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
    pMainLayout->addLayout(pLayout);
    pMainLayout->addSpacing(5);
    pMainLayout->addLayout(pBtnLayout);
}

void CWorkflowPublishDlg::initConnections()
{
    connect(m_pOkBtn, &QPushButton::clicked, this, &CWorkflowPublishDlg::validate);
    connect(m_pCancelBtn, &QPushButton::clicked, this, &CWorkflowPublishDlg::reject);
    connect(m_pRadioExistingProject, &QRadioButton::toggled, [&](bool bChecked){
        bChecked ? m_pProjectStackWidget->setCurrentIndex(0) : m_pProjectStackWidget->setCurrentIndex(1);
    });
}

QString CWorkflowPublishDlg::getProjectDisplayName(const QJsonObject& project) const
{
    auto path = project["path"].toString();
    auto name = project["name"].toString();
    QStringList pathItems = path.split("/", Qt::SkipEmptyParts);

    if (pathItems.size() > 2)
    {
        name += " ( ";
        for (int i=1; i<pathItems.size() - 1; ++i)
            name += pathItems[i] + "/";
        name += " )";
    }
    return name;
}

QJsonObject CWorkflowPublishDlg::getProjectFromPath(const QString &path) const
{
    for (int i=0; i<m_projects.size(); ++i)
    {
        QJsonObject project = m_projects[i].toObject();
        if (project["path"].toString() == path)
            return project;
    }
    return QJsonObject();
}

void CWorkflowPublishDlg::validate()
{
    m_workflowName = m_pEditWfName->text();
    if (m_workflowName.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Enter a workflow name please."));
        msgBox.exec();
        return;
    }

    m_workflowDescription = m_pEditDescription->toPlainText();
    if (m_workflowName.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Enter a description for the workflow please."));
        msgBox.exec();
        return;
    }

    if (m_pRadioNewProject->isChecked())
    {
        if (m_pNewProjectName->text().isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText(tr("Enter a project name please."));
            msgBox.exec();
            return;
        }
    }

    emit accept();
}
