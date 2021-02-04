#ifndef CSCENARIOSIMPLEIMAGE_HPP
#define CSCENARIOSIMPLEIMAGE_HPP

#include "Model/Wizard/CWizardScenario.h"

class CScenarioSimpleImage : public CWizardScenario
{
        Q_OBJECT
    public:
        CScenarioSimpleImage(CMainModel* pModel, CMainView* pView) : CWizardScenario(pModel, pView)
        {
            makeScenario();
        }

    public slots:
        void onCheckImgprocFolder(const QModelIndex& index)
        {
            auto pModel = static_cast<const CProcessProxyModel*>(index.model());
            auto srcIndex = pModel->mapToSource(index);
            auto pItem = static_cast<CProcessModel::TreeItem*>(srcIndex.internalPointer());
            if(pItem->getTypeId() == TreeItemType::FOLDER && pItem->getName() == "Image processing")
                emit doNext();
        }
        void onCheckBilateralFilter(const QModelIndex& index)
        {
            auto pModel = static_cast<const CProcessTableProxyModel*>(index.model());
            auto srcIndex = pModel->mapToSource(index);
            auto pTable = static_cast<const QSqlTableModel*>(srcIndex.model());
            // Get name from table process (id, name, description, folder_id)
            QString name = pTable->record(index.row()).value(1).toString();
            if(name == "Bilateral Filter")
                emit doNext();
        }

    protected:
        virtual void makeScenario()
        {
            addStep(m_pView->getBtn(CMainView::BTN_OPEN_IMG),
                    "Open your first image",
                    "Please choose one or more images.",
                    "",
                    m_pModel->getProjectManager(),
                    SIGNAL(doUpdateIndex(const QModelIndex&)));
            addStep(m_pView->getBtn(CMainView::BTN_PROCESS_PANE),
                    "Open process list",
                    "Please open our process library and navigate through all our algorithms.",
                    "",
                    m_pView->getBtn(CMainView::BTN_PROCESS_PANE),
                    SIGNAL(clicked(bool)));
            addStep(m_pView->getProcessPane()->getTreeView(),
                    "Click on Image processing folder.",
                    "Please select this folder which contains some useful algorithms.",
                    "QTreeView{border: 2px solid green; border-radius: 5px}",
                    m_pView->getProcessPane()->getTreeView(),
                    SIGNAL(clicked(QModelIndex)),
                    SLOT(onCheckImgprocFolder(QModelIndex)));
            addStep(m_pView->getProcessPane()->getListPopup()->getListView(),
                    "Choose the Bilateral filter.",
                    "This process smoothes your image with an edge-preserving filter.",
                    "QListView{border: 2px solid green; border-radius: 5px}",
                    m_pView->getProcessPane()->getListPopup()->getListView(),
                    SIGNAL(clicked(QModelIndex)),
                    SLOT(onCheckBilateralFilter(QModelIndex)));
            addStep(m_pView->getProcessPane()->getProcessParameters(),
                    "Click on the Apply button",
                    "Click and watch the result on the right.",
                    "QPushButton{border: 2px solid green; border-radius: 5px}",
                    m_pView->getProcessPane(),
                    SIGNAL(doApplyProcess(std::string,std::shared_ptr<CProtocolTaskParam>)));
        }
};

class CScenarioSimpleImageFactory : public CWizardScenarioFactory
{
    public:

        CScenarioSimpleImageFactory()
        {
            m_info.m_name = "Bilateral filter";
            m_info.m_description = "This tutorial explains how to use the process library to apply a bilateral filter on your image.";
            m_info.m_authors = "Guillaume Demarcq";
            m_info.m_keywords = "bilateral, process, begin, image, start, howto";
            m_info.m_iconPath = "";
        }

        virtual CWizardScenario* create(CMainModel* pModel, CMainView* pView) override
        {
            return new CScenarioSimpleImage(pModel, pView);
        }
};

#endif // CSCENARIOSIMPLEIMAGE_HPP
