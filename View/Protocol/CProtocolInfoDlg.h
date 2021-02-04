#ifndef CPROTOCOLINFODLG_H
#define CPROTOCOLINFODLG_H

#include "View/Common/CDialog.h"

class CProtocolInfoDlg : public CDialog
{
    Q_OBJECT

    public:

        CProtocolInfoDlg(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void        setName(const QString& name);
        void        setDescription(const QString& text);

        QString     getName() const;

    signals:

        void        doLoadProtocol();

    private:

        void        initLayout();
        void        initConnections();

        QFrame*     createLine();

    private:

        QLabel*         m_pLabel = nullptr;
        QLabel*         m_pLabelDescription = nullptr;
        QPushButton*    m_pLoadBtn = nullptr;
};

#endif // CPROTOCOLINFODLG_H
