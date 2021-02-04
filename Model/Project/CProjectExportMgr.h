#ifndef CPROJECTEXPORT_H
#define CPROJECTEXPORT_H

class CProjectModel;

class CProjectExportMgr
{
    public:

        CProjectExportMgr(CProjectModel* pModel, const QString& folder);

        void    run();

    private:

        void        copy(const QModelIndex &srcIndex, const QModelIndex &dstIndex);
        QModelIndex addItem(const QModelIndex& srcIndex, const QModelIndex& dstParent);

    private:

        CProjectModel*  m_pSrcModel = nullptr;
        CProjectModel*  m_pNewModel = nullptr;
        std::string     m_parentFolder;
        std::string     m_projectFolder;
        std::string     m_dataFolder;
};

#endif // CPROJECTEXPORT_H
