#ifndef CPROTOCOLNEWDLG_H
#define CPROTOCOLNEWDLG_H

#include "View/Common/CDialog.h"
#include "Main/forwards.hpp"

class CProtocolNewDlg : public CDialog
{
    public:

        CProtocolNewDlg(const QString &name, const QStringList& names, QWidget * parent = 0, Qt::WindowFlags f = 0);

        QString         getName() const;
        QString         getKeywords() const;
        QString         getDescription() const;

    private:

        void            initLayout();
        void            initConnections();

        void            validate();

    private:

        QLineEdit*      m_pEditName = nullptr;
        QPlainTextEdit* m_pEditKeywords = nullptr;
        QPlainTextEdit* m_pEditDescription = nullptr;
        QPushButton*    m_pBtnOk = nullptr;
        QPushButton*    m_pBtnCancel = nullptr;
        QString         m_name;
        QString         m_keywords;
        QString         m_description;
        QStringList     m_protocolNames;
};

#endif // CPROTOCOLNEWDLG_H
