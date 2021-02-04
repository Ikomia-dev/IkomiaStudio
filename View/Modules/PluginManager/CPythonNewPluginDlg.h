#ifndef CPYTHONNEWPLUGINWIDGET_H
#define CPYTHONNEWPLUGINWIDGET_H

#include <QWidget>
#include "View/Common/CDialog.h"

class CPythonNewPluginDlg : public CDialog
{
    Q_OBJECT

    public:

        explicit CPythonNewPluginDlg(QWidget *parent = nullptr, Qt::WindowFlags f = 0);

        QString getName() const;

    private slots:

        void    onGeneratePlugin();

    private:

        void    initLayout();
        void    initConnections();

    private:

        QLineEdit*      m_pEditPyName = nullptr;
        QComboBox*      m_pComboPyProcessType = nullptr;
        QComboBox*      m_pComboPyWidgetType = nullptr;
        QComboBox*      m_pComboPyQt = nullptr;
        QPushButton*    m_pBtnGenerate = nullptr;
        QPushButton*    m_pBtnCancel = nullptr;
        QString         m_name;
};

#endif // CPYTHONNEWPLUGINWIDGET_H
