#ifndef CWORKFLOWPUBLISHDLG_H
#define CWORKFLOWPUBLISHDLG_H

#include "View/Common/CDialog.h"
#include "View/Common/CResizeStackedWidget.h"
#include "Model/User/CUser.h"

class CWorkflowPublishDlg: public CDialog
{
    Q_OBJECT

    public:

        CWorkflowPublishDlg(const QString& wfName, const QString& wfDescription,
                            const QJsonArray &projects,
                            const CUser& user,
                            QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        QString     getWorkflowName() const;
        QString     getWorkflowDescription() const;
        QString     getProjectName() const;
        QString     getProjectDescription() const;
        QString     getNamespacePath() const;

        bool        isNewProject() const;

    private:

        void        initLayout(const CUser &user);
        void        initConnections();

        QString     getProjectDisplayName(const QJsonObject &project) const;
        QJsonObject getProjectFromPath(const QString& path) const;

    private slots:

        void        validate();

    private:

        QString                 m_workflowName;
        QString                 m_workflowDescription;
        QJsonArray              m_projects;
        QLineEdit*              m_pEditWfName = nullptr;
        QLineEdit*              m_pNewProjectName = nullptr;
        QPlainTextEdit*         m_pEditDescription = nullptr;
        QPlainTextEdit*         m_pNewProjectDescription = nullptr;
        QRadioButton*           m_pRadioNewProject = nullptr;
        QRadioButton*           m_pRadioExistingProject = nullptr;
        QComboBox*              m_pComboNamespace = nullptr;
        QComboBox*              m_pComboProject = nullptr;
        QPushButton*            m_pOkBtn = nullptr;
        QPushButton*            m_pCancelBtn = nullptr;
        CResizeStackedWidget*   m_pProjectStackWidget = nullptr;
};

#endif // CWORKFLOWPUBLISHDLG_H
