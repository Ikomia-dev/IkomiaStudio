#ifndef CDATASETLOADPOLICYDLG_H
#define CDATASETLOADPOLICYDLG_H

#include "View/Common/CDialog.h"
#include "Main/CoreDefine.hpp"
#include "Main/forwards.hpp"

class CDatasetLoadPolicyDlg : public CDialog
{
    public:

        CDatasetLoadPolicyDlg(TreeItemType srcType, QWidget * parent = 0, Qt::WindowFlags f = 0);

        DatasetLoadPolicy   getLoadPolicy();

    private:

        void                initLayout();
        void                initPolicyWidget();
        void                initDimensionWidget();
        void                initConnections();

    private slots:

        void                onToggledMulti(bool bChecked);
        void                onToggledSingle(bool bChecked);
        void                onToggledStructured(bool bChecked);
        void                onToggledSimpleList(bool bChecked);
        void                onToggledVolume(bool bChecked);
        void                onToggledPosition(bool bChecked);
        void                onToggledTime(bool bChecked);
        void                onToggledModality(bool bChecked);

    private:

        QWidget*            m_pPolicyWidget = nullptr;
        QRadioButton*       m_pRadioMulti = nullptr;
        QRadioButton*       m_pRadioSingle = nullptr;
        QRadioButton*       m_pRadioStructured = nullptr;
        QWidget*            m_pDimensionWidget = nullptr;
        QRadioButton*       m_pRadioList = nullptr;
        QRadioButton*       m_pRadioVolume = nullptr;
        QRadioButton*       m_pRadioPosition = nullptr;
        QRadioButton*       m_pRadioTime = nullptr;
        QRadioButton*       m_pRadioModality = nullptr;
        QPushButton*        m_pOkBtn = nullptr;
        QPushButton*        m_pCancelBtn = nullptr;
        Relationship        m_relationship = Relationship::MANY_TO_ONE;
        DataDimension       m_extraDim = DataDimension::NONE;
        TreeItemType        m_srcType = TreeItemType::NONE;
};

#endif // CDATASETLOADPOLICYDLG_H
