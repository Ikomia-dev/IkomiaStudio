#include "CProcessEditDocFrame.h"
#include "View/Common/CDialog.h"
#include <QtWidgets>
#include "Main/AppTools.hpp"

CProcessEditDocFrame::CProcessEditDocFrame(QWidget *parent, Qt::WindowFlags f)
    : QFrame(parent, f)
{
    initLayout();
}

void CProcessEditDocFrame::setCurrentUser(const CUser &user)
{
    m_currentUser = user;
}

void CProcessEditDocFrame::setProcessInfo(const CProcessInfo &info)
{
    m_originalInfo = info;
    m_bFullEdit = info.m_bInternal == false && (info.m_userId == -1 || info.m_userId == m_currentUser.m_id);
    fillForm();
    m_bFullEdit == true ? m_pStackWidget->setCurrentIndex(0) : m_pStackWidget->setCurrentIndex(1);
}

void CProcessEditDocFrame::initLayout()
{
    auto pFullEditWidget = createFullEditWidget();
    auto pPartialEditWidget = createPartialEditWidget();

    QPushButton* pSaveBtn = new QPushButton(tr("Save"));
    pSaveBtn->setFixedWidth(100);
    connect(pSaveBtn, &QPushButton::clicked, this, &CProcessEditDocFrame::onSave);

    QPushButton* pCancelBtn = new QPushButton(tr("Cancel"));
    pCancelBtn->setFixedWidth(100);
    connect(pCancelBtn, &QPushButton::clicked, [&]{ emit doCancel(); });

    QHBoxLayout* pBtnLayout = new QHBoxLayout;
    pBtnLayout->setAlignment(Qt::AlignCenter);
    pBtnLayout->addWidget(pSaveBtn);
    pBtnLayout->addWidget(pCancelBtn);

    m_pStackWidget = new QStackedWidget;
    m_pStackWidget->addWidget(pFullEditWidget);
    m_pStackWidget->addWidget(pPartialEditWidget);

    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->setContentsMargins(0, 0, 0, 0);
    pMainLayout->addWidget(m_pStackWidget);
    pMainLayout->addLayout(pBtnLayout);

    setLayout(pMainLayout);
}

QWidget *CProcessEditDocFrame::createFullEditWidget()
{
    QLabel* pLabelDescription =  new QLabel(tr("Description"));
    m_pTextEditDescription = new QTextEdit;
    m_pTextEditDescription->setFixedHeight(150);

    QLabel* pLabelKeywords =  new QLabel(tr("Keywords"));
    m_pTextEditKeywords = new QTextEdit;
    m_pTextEditKeywords->setFixedHeight(40);

    QLabel* pLabelAuthors = new QLabel(tr("Authors"));
    m_pTextEditAuthors = new QTextEdit;
    m_pTextEditAuthors->setFixedHeight(40);

    QLabel* pLabelArticle = new QLabel(tr("Article"));
    m_pTextEditArticle = new QTextEdit;
    m_pTextEditArticle->setFixedHeight(40);

    QLabel* pLabelJournal = new QLabel(tr("Journal"));
    m_pTextEditJournal = new QTextEdit;
    m_pTextEditJournal->setFixedHeight(40);

    QLabel* pLabelYear = new QLabel(tr("Year"));
    m_pSpinYear = new QSpinBox;
    m_pSpinYear->setRange(-1, 2050);
    m_pSpinYear->setFixedWidth(100);

    QLabel* pLabelDocLink = new QLabel(tr("Documentation link"));
    m_pEditDocLink = new QLineEdit;

    QLabel* pLabelVersion = new QLabel(tr("Version"));
    m_pEditVersion = new QLineEdit;

    /*QLabel* pLabelOS = new QLabel(tr("Operating system"));
    m_pRadioLinux = new QRadioButton(tr("Linux"));
    m_pRadioMac = new QRadioButton(tr("Mac"));
    m_pRadioWin = new QRadioButton(tr("Win"));*/

    QLabel* pLabelIconPath = new QLabel(tr("Icon path"));
    m_pEditIconPath = new QLineEdit;
    QPushButton* pBrowseBtn = new QPushButton("...");
    connect(pBrowseBtn, &QPushButton::clicked, this, &CProcessEditDocFrame::onBrowseIconPath);

    QGridLayout* pLayout = new QGridLayout;
    pLayout->addWidget(pLabelDescription, 0, 0, 1, 1);
    pLayout->addWidget(m_pTextEditDescription, 0, 1, 1, 2);
    pLayout->addWidget(pLabelKeywords, 1, 0, 1, 1);
    pLayout->addWidget(m_pTextEditKeywords, 1, 1, 1, 2);
    pLayout->addWidget(pLabelAuthors, 2, 0, 1, 1);
    pLayout->addWidget(m_pTextEditAuthors, 2, 1, 1, 2);
    pLayout->addWidget(pLabelArticle, 3, 0, 1, 1);
    pLayout->addWidget(m_pTextEditArticle, 3, 1, 1, 2);
    pLayout->addWidget(pLabelJournal, 4, 0, 1, 1);
    pLayout->addWidget(m_pTextEditJournal, 4, 1, 1, 2);
    pLayout->addWidget(pLabelYear, 5, 0, 1, 1);
    pLayout->addWidget(m_pSpinYear, 5, 1, 1, 1);
    pLayout->addWidget(pLabelDocLink, 6, 0, 1, 1);
    pLayout->addWidget(m_pEditDocLink, 6, 1, 1, 2);
    pLayout->addWidget(pLabelVersion, 7, 0, 1, 1);
    pLayout->addWidget(m_pEditVersion, 7, 1, 1, 2);
    pLayout->addWidget(pLabelIconPath, 8, 0, 1, 1);
    pLayout->addWidget(m_pEditIconPath, 8, 1, 1, 1);
    pLayout->addWidget(pBrowseBtn, 9, 2, 1, 1);
    /*pLayout->addWidget(pLabelOS, 10, 0, 1, 1);
    pLayout->addWidget(m_pRadioLinux, 11, 1, 1, 1);
    pLayout->addWidget(m_pRadioMac, 12, 1, 1, 1);
    pLayout->addWidget(m_pRadioWin, 13, 1, 1, 1);*/

    QWidget* pFormWidget = new QWidget;
    pFormWidget->setLayout(pLayout);
    return pFormWidget;
}

QWidget *CProcessEditDocFrame::createPartialEditWidget()
{
    QLabel* pLabelKeywords =  new QLabel(tr("Keywords"));
    m_pTextEditKeywords2 = new QTextEdit;
    m_pTextEditKeywords2->setFixedHeight(40);

    QGridLayout* pLayout = new QGridLayout;
    pLayout->addWidget(pLabelKeywords, 0, 0, 1, 1, Qt::AlignTop);
    pLayout->addWidget(m_pTextEditKeywords2, 0, 1, 1, 2, Qt::AlignTop);

    QWidget* pFormWidget = new QWidget;
    pFormWidget->setLayout(pLayout);
    return pFormWidget;
}

void CProcessEditDocFrame::fillForm()
{
    if(m_bFullEdit == true)
    {
        m_pTextEditDescription->setPlainText(QString::fromStdString(m_originalInfo.m_description));
        m_pTextEditKeywords->setPlainText(QString::fromStdString(m_originalInfo.m_keywords));
        m_pTextEditAuthors->setPlainText(QString::fromStdString(m_originalInfo.m_authors));
        m_pTextEditArticle->setPlainText(QString::fromStdString(m_originalInfo.m_article));
        m_pTextEditJournal->setPlainText(QString::fromStdString(m_originalInfo.m_journal));
        m_pSpinYear->setValue(m_originalInfo.m_year);
        m_pEditDocLink->setText(QString::fromStdString(m_originalInfo.m_docLink));
        m_pEditVersion->setText(QString::fromStdString(m_originalInfo.m_version));
        m_pEditIconPath->setText(QString::fromStdString(m_originalInfo.m_iconPath));
        /*m_pRadioLinux->setChecked(m_originalInfo.m_os == CProcessInfo::LINUX);
        m_pRadioMac->setChecked(m_originalInfo.m_os == CProcessInfo::OSX);
        m_pRadioWin->setChecked(m_originalInfo.m_os == CProcessInfo::WIN);*/
    }
    else
        m_pTextEditKeywords2->setPlainText(QString::fromStdString(m_originalInfo.m_keywords));
}

void CProcessEditDocFrame::onSave()
{
    CProcessInfo info;
    info.m_id = m_originalInfo.m_id;
    info.m_name = m_originalInfo.m_name;
    info.m_os = m_originalInfo.m_os;

    if(m_bFullEdit == true)
    {
        info.m_description = m_pTextEditDescription->toPlainText().toStdString();
        info.m_keywords = m_pTextEditKeywords->toPlainText().toStdString();
        info.m_authors = m_pTextEditAuthors->toPlainText().toStdString();
        info.m_article = m_pTextEditArticle->toPlainText().toStdString();
        info.m_journal = m_pTextEditJournal->toPlainText().toStdString();
        info.m_year = m_pSpinYear->value();
        info.m_docLink = m_pEditDocLink->text().toStdString();
        info.m_version = m_pEditVersion->text().toStdString();
        info.m_iconPath = m_pEditIconPath->text().toStdString();
        //info.m_os = m_pRadioLinux->isChecked() ? CProcessInfo::LINUX : (m_pRadioMac->isChecked() ? CProcessInfo::OSX : CProcessInfo::WIN);
    }
    else
        info.m_keywords = m_pTextEditKeywords2->toPlainText().toStdString();

    emit doSave(m_bFullEdit, info);
}

void CProcessEditDocFrame::onBrowseIconPath()
{
    auto fileName = Utils::File::saveFile(this, tr("Icon"), "", tr("All images (*.jpg *.jpeg *.tif *.tiff *.png *.bmp)"), QStringList({"jpg", "jpeg", "tif", "tiff", "png", "bmp"}), ".png");
    if(fileName.isEmpty())
        return;

    m_pEditIconPath->setText(fileName);
}
