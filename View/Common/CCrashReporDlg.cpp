#include "CCrashReporDlg.h"
#include "Model/Crash/QBreakpadHandler.h"
#include "Model/Crash/QBreakpadHttpUploader.h"
#include <QHttpPart>
#include <QNetworkReply>

CCrashReporDlg::CCrashReporDlg(QWidget* parent, Qt::WindowFlags f) : CDialog(tr("Crash report"), parent, LABEL | RESIZABLE | MOVABLE | EFFECT_ENABLED, f)
{
    setWindowFlag(Qt::WindowStaysOnTopHint);
    init();
}

void CCrashReporDlg::setProgressBarMaximum(qint64 totalBytes)
{
    m_pProgress->setMaximum(totalBytes);
}

void CCrashReporDlg::onSendDumpFiles()
{
    connect(QBreakpadInstance.getUploader(), &QBreakpadHttpUploader::uploadProcess, this, &CCrashReporDlg::onUpdateProgress);
    connect(QBreakpadInstance.getUploader(), &QBreakpadHttpUploader::finished, [&]{ QDialog::accept(); });
    connect(QBreakpadInstance.getUploader(), &QBreakpadHttpUploader::error, this, &CCrashReporDlg::onError);

    // Send dump file through QBreakpadInstance
    QBreakpadInstance.sendDumps();
}

void CCrashReporDlg::onQuit()
{
    // Delete dump files through QBreakpadInstance
    deleteDumpFiles();
    QDialog::accept();
}

void CCrashReporDlg::onUpdateProgress(qint64 sentBytes, qint64 totalBytes)
{
    m_pProgress->setValue(static_cast<int>(sentBytes));
    m_pProgress->setMaximum(static_cast<int>(totalBytes));
}

void CCrashReporDlg::onError(const QString& err)
{
    QMessageBox msgBox(QMessageBox::Warning, tr("Warning"), err, 0, this);
    msgBox.addButton(tr("Retry &Again"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("&Send later"), QMessageBox::RejectRole);

    if (msgBox.exec() == QMessageBox::AcceptRole)
        QBreakpadInstance.sendDumps();
    else
        QDialog::reject();
}

void CCrashReporDlg::deleteDumpFiles() const
{
    auto path = QBreakpadInstance.dumpPath();
    if(path.isEmpty())
        return;

    auto list = QBreakpadInstance.dumpFileList();
    for(auto it : list)
    {
        QFile file(path+"/"+it);
        file.remove();
    }
}

void CCrashReporDlg::init()
{
    auto pLayout = getContentLayout();

    m_pTextBrowser = new QTextBrowser;
    initInformation();

    QPushButton* pSendBtn = new QPushButton(tr("Send"));
    QPushButton* pQuitBtn = new QPushButton(tr("Quit"));

    QHBoxLayout* pHBox = new QHBoxLayout;
    pHBox->addWidget(pSendBtn);
    pHBox->addWidget(pQuitBtn);

    m_pExplainLabel = new QLabel(tr("Ikomia has crashed during the last session, would you like to send us the crash report?"));
    m_pProgress = new QProgressBar;
    m_pProgress->setValue(0);
    m_pProgress->setMaximum(0);

    pLayout->addWidget(m_pExplainLabel);
    pLayout->addWidget(m_pTextBrowser);
    pLayout->addWidget(m_pProgress);
    pLayout->addLayout(pHBox);

    connect(pSendBtn, &QPushButton::clicked, this, &CCrashReporDlg::onSendDumpFiles);
    connect(pQuitBtn, &QPushButton::clicked, this, &CCrashReporDlg::onQuit);
}

void CCrashReporDlg::initInformation()
{
    QString detailText;

    detailText.append(tr("We specifically send the following information:\n\n"));

    auto list = QBreakpadInstance.dumpFileList();

    detailText.append(QString("ProductName: %1\n").arg(QCoreApplication::applicationName()));
    detailText.append(QString("Version: %1\n").arg(QCoreApplication::applicationVersion()));

    for(auto it : list)
        detailText.append(QString("Dump files: %1\n").arg(it));
    detailText.append(QString("Log file: %1\n").arg("log.txt"));

    m_pTextBrowser->setText(detailText);
}
