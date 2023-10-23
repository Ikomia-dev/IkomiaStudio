#ifndef CWORKFLOWPACKAGE_H
#define CWORKFLOWPACKAGE_H


class CWorkflowPackage
{
    public:

        CWorkflowPackage(const QString& wfPath);
        ~CWorkflowPackage();

        QString create();

    private:

        void    addAlgorithms();

        bool    updatePathInParameters(QJsonObject &data);

    private:

        QString         m_path;
        QString         m_packageDir;
        QJsonObject     m_workflow;
};

#endif // CWORKFLOWPACKAGE_H
