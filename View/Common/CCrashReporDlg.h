#ifndef CCRASHREPORDLG_H
#define CCRASHREPORDLG_H

#include "CDialog.h"
#include <QNetworkReply>

class CCrashReporDlg : public CDialog
{
    public:
        explicit CCrashReporDlg(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

        void    setProgressBarMaximum(qint64 totalBytes);

    public slots:
        void    onSendDumpFiles();
        void    onQuit();
        void    onUpdateProgress(qint64 sentBytes, qint64 totalBytes);
        void    onError(const QString& err);

    private:
        void    deleteDumpFiles() const;
        void    init();
        void    initInformation();

    private:
        QLabel*         m_pExplainLabel = nullptr;
        QTextBrowser*   m_pTextBrowser = nullptr;
        QProgressBar*   m_pProgress = nullptr;
};

#endif // CCRASHREPORDLG_H
