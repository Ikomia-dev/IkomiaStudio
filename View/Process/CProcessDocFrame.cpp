#include "CProcessDocFrame.h"
#include "UtilsTools.hpp"

CProcessDocFrame::CProcessDocFrame(QWidget *parent, Qt::WindowFlags f):
    QFrame(parent, f)
{
    initLayout();
}

void CProcessDocFrame::setProcessInfo(const CProcessInfo &info)
{
    QString mdContent = generateMarkdown(info);
    m_pDoc->setMarkdown(mdContent);
}

void CProcessDocFrame::initLayout()
{
    setProperty("class", "CProcessDocFrame");

    QHBoxLayout* pMainLayout = new QHBoxLayout;
    pMainLayout->setContentsMargins(0, 0, 0, 0);

    m_pBrowser = new QTextBrowser;
    m_pDoc = new QTextDocument;
    m_pBrowser->setOpenExternalLinks(true);
    m_pBrowser->setDocument(m_pDoc);
    pMainLayout->addWidget(m_pBrowser);

    setLayout(pMainLayout);
}

QString CProcessDocFrame::getMarkdownTemplate() const
{
    return QString("_icon_ \n\n"
                   "<span style=\"color:#cc5a20\"><h1> _name_ </h1></span> ![Language logo](_languageIcon_)  \n\n"
                   "**Version**: _version_  \n"
                   "**_modifiedTxt_**: _modified_  \n"
                   "**_createdTxt_**: _created_  \n"
                   "**_statusLabel_**: _status_  \n"
                   "_description_\n\n"
                   "_docLink_ \n\n"
                   "<span style=\"color:#cc5a20\"><h2> Publication </h2></span> \n\n"
                   "_article_ \n\n"
                   "*_authors_* \n\n"
                   "_journal_ \n\n"
                   "_year_ \n\n"
                   "_repo_ \n\n"
                   "_license_"
                   "<span style=\"color:#cc5a20\"><h2> _keywordsTxt_ </h2></span> \n\n"
                   "_keywords_");
}

QString CProcessDocFrame::generateMarkdown(const CProcessInfo &info) const
{
    auto templateContent = getMarkdownTemplate();

    QString languageIconPath;
    if(info.m_language == CProcessInfo::PYTHON)
        languageIconPath = "qrc:/Images/python-language-logo-32.png";
    else
        languageIconPath = "qrc:/Images/C++-language-logo-32.png";

    //Process name
    auto newContent = templateContent.replace("_name_", QString::fromStdString(info.m_name));

    //Language icon
    newContent = newContent.replace("_languageIcon_", languageIconPath);

    //Process icon
    if(info.m_iconPath.empty() == false && info.m_iconPath.at(0) != ':')
        newContent = newContent.replace("_icon_", QString("![Process icon >](file:%1)").arg(QString::fromStdString(info.m_iconPath)));
    else
        newContent = newContent.replace("_icon_", "");

    //Version
    newContent = newContent.replace("_version_", QString::fromStdString(info.m_version));

    //Modified date
    QString modifiedTxt = tr("Modified");
    newContent = newContent.replace("_modifiedTxt_", modifiedTxt);
    newContent = newContent.replace("_modified_", QString::fromStdString(info.m_modifiedDate));

    //Created date
    QString createdTxt = tr("Created");
    newContent = newContent.replace("_createdTxt_", createdTxt);
    newContent = newContent.replace("_created_", QString::fromStdString(info.m_createdDate));

    //Status: deprecated or not
    QString status;
    QString ikomiaVersion = QString::fromStdString(info.m_ikomiaVersion);
    newContent = newContent.replace("_statusLabel_", tr("Status"));

    if(Utils::IkomiaApp::isDeprecated(ikomiaVersion))
    {
        status = tr("<span style=\"color:#9a0000\">deprecated</span>(based on Ikomia version %1 while the current is %2)")
                .arg(ikomiaVersion)
                .arg(Utils::IkomiaApp::getCurrentVersionNumber());
    }
    else
    {
        status = tr("<span style=\"color:#008f00\">OK</span> (based on Ikomia version %1)").arg(ikomiaVersion);
    }
    newContent = newContent.replace("_status_", status);

    //Description
    newContent = newContent.replace("_description_", QString::fromStdString(info.m_description));

    //Documentation link
    if(info.m_docLink.empty() == false)
    {
        QString link = QString("[%1](%2)").arg(tr("Online documentation")).arg(QString::fromStdString(info.m_docLink));
        newContent = newContent.replace("_docLink_", link);
    }
    else
        newContent = newContent.replace("_docLink_", "");

    //Publication
    QString emphaseCode = "";
    if(info.m_article.empty() == false)
        emphaseCode = "**";

    newContent = newContent.replace("_article_", emphaseCode + QString::fromStdString(info.m_article) + emphaseCode);
    newContent = newContent.replace("_journal_", QString::fromStdString(info.m_journal));
    newContent = newContent.replace("_authors_", QString::fromStdString(info.m_authors));

    if(info.m_year > 0)
        newContent = newContent.replace("_year_", QString::number(info.m_year));
    else
        newContent = newContent.replace("_year_", "");

    if(info.m_repo.empty() == false)
    {
        auto repoStr = QString("<span style=\"color:#cc5a20\"><h2> Repository </h2></span> \n\n [%1](%1) \n\n").arg(QString::fromStdString(info.m_repo));
        newContent = newContent.replace("_repo_", repoStr);
    }
    else
        newContent = newContent.replace("_repo_", "");

    if(info.m_license.empty() == false)
    {
        auto licenceStr = QString("<span style=\"color:#cc5a20\"><h2> License </h2></span> \n\n %1 \n\n").arg(QString::fromStdString(info.m_license));
        newContent = newContent.replace("_license_", licenceStr);
    }
    else
        newContent = newContent.replace("_license_", "");

    //Keywords
    QString keywordsTxt = tr("Keywords");
    newContent = newContent.replace("_keywordsTxt_", keywordsTxt);
    newContent = newContent.replace("_keywords_", QString::fromStdString(info.m_keywords));

    return newContent;
}
