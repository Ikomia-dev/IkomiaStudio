#ifndef CWORKFLOWEXPOSEPARAMSDLG_H
#define CWORKFLOWEXPOSEPARAMSDLG_H

#include "View/Common/CDialog.h"
#include "Core/CWorkflow.h"


class CWorkflowExposeParamsDlg: public CDialog
{
    Q_OBJECT

    public:

        CWorkflowExposeParamsDlg(const WorkflowTaskPtr& taskPtr,
                                 const WorkflowVertex &taskId,
                                 const CWorkflow::ExposedParams& params,
                                 QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        CWorkflow::ExposedParams    getExposedParams() const;

    private:

        void        initLayout();
        void        initConnections();

        void        fillParamsTable(const WorkflowTaskPtr& taskPtr);

        std::pair<bool, CWorkflowParam> getExposedParam(const std::string& paramName);

    private slots:

        void        validate();

    private:

        QTableWidget*               m_pParamsTable = nullptr;
        QPushButton*                m_pOkBtn = nullptr;
        QPushButton*                m_pCancelBtn = nullptr;
        WorkflowVertex              m_taskId;
        CWorkflow::ExposedParams    m_workflowParams;
        CWorkflow::ExposedParams    m_taskParams;
};

#endif // CWORKFLOWEXPOSEPARAMSDLG_H
