#ifndef CIMAGEEXPORTDLG_H
#define CIMAGEEXPORTDLG_H

#include "View/Common/CDialog.h"

class CImageExportDlg: public CDialog
{
    public:

        CImageExportDlg(QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        QString     getFileName() const;
        bool        isGraphicsExported() const;

    private:

        void        initLayout();
        void        initConnections();

    private slots:

        void        onBrowse();
        void        onValidate();

    private:

        QCheckBox*      m_pCheckBurnGraphics = nullptr;
        QLineEdit*      m_pEditPath = nullptr;
        QPushButton*    m_pBrowseBtn = nullptr;
        QPushButton*    m_pOkBtn = nullptr;
        QPushButton*    m_pCancelBtn = nullptr;
};

#endif // CIMAGEEXPORTDLG_H
