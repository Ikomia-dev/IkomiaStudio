#ifndef CPROTOCOLINPUTTYPEDLG_H
#define CPROTOCOLINPUTTYPEDLG_H

#include <QObject>
#include "View/Common/CDialog.h"
#include "Main/AppTools.hpp"
#include "Model/Protocol/CProtocolManager.h"

class QTreeView;
class CProjectDataProxyModel;

class CProtocolInputTypeDlg : public CDialog
{
    Q_OBJECT

    public:

        CProtocolInputTypeDlg(QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void                setModel(CProjectDataProxyModel* pModel);

        CProtocolInput      getInput() const;

    public slots:

        int                 exec();


    private slots:

        void                onInputTypeSelected(QListWidgetItem *current, QListWidgetItem *previous);
        void                onBack();
        void                onValidate();

    signals:

        void                doQueryProjectDataProxyModel(const TreeItemType& inputType, const std::vector<DataDimension>& filters);

    private:

        void                initLayout();
        void                initInputTypesWidget();
        void                initInputOptionWidget();
        void                initProjectViewWidget();
        void                initConnections();

        void                showInputTypes();
        void                showInputOptions();
        void                showProjectView();

        void                validateFilters();
        void                validateProjectItems();

    private:

        QStackedWidget*         m_pStackedWidget = nullptr;
        QListWidget*            m_pInputTypesWidget = nullptr;
        QWidget*                m_pInputOptionWidget = nullptr;
        QWidget*                m_pProjectViewWidget = nullptr;
        QTreeView*              m_pTreeView = nullptr;

        QPushButton*            m_pBackBtn = nullptr;
        QPushButton*            m_pNextBtn = nullptr;
        QPushButton*            m_pCancelBtn = nullptr;
        CProjectDataProxyModel* m_pModel = nullptr;

        QCheckBox*              m_pCheckSimpleDataset = nullptr;
        QCheckBox*              m_pCheckVolumeDataset = nullptr;
        QCheckBox*              m_pCheckTimeDataset = nullptr;

        CProtocolInput          m_input;
};

#endif // CPROTOCOLINPUTTYPEDLG_H
