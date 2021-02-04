#include "CStaticDisplay.h"
#include <QLabel>

CStaticDisplay::CStaticDisplay(const QString &name, QWidget *parent) : CDataDisplay(parent, name)
{
    initLayout();
    m_typeId = DisplayType::EMPTY_DISPLAY;
}

CStaticDisplay::CStaticDisplay(const QString &text, const QString &name, QWidget *parent) : CDataDisplay(parent, name)
{
    initLayout(text);
    m_typeId = DisplayType::EMPTY_DISPLAY;
}

void CStaticDisplay::setFont(int pointSize, int weight, bool bItalic)
{
    //Set font properties (size) with style sheet to overwrite application settings
    QString fontSize = QString::number(pointSize) + "px";

    QString fontWeight;
    switch(weight)
    {
        case QFont::Normal: fontWeight = "normal"; break;
        case QFont::Bold: fontWeight = "bold"; break;
        default: fontWeight = QString::number(weight); break;
    }

    QString fontStyle = "normal";
    if(bItalic)
        fontStyle = "italic";

    QRegularExpression re;
    re.setPattern("font-size:.+?;");
    m_labelStyle.replace(re, QString("font-size: %1;").arg(fontSize));
    re.setPattern("font-weight:.+?;");
    m_labelStyle.replace(re, QString("font-weight: %1;").arg(fontWeight));
    re.setPattern("font-style:.+?;");
    m_labelStyle.replace(re, QString("font-style: %1;").arg(fontStyle));
    m_pLabel->setStyleSheet(m_labelStyle);
}

void CStaticDisplay::setBackgroundColor(QColor color)
{
    QRegularExpression re;
    re.setPattern("background-color:.+?;");
    m_style.replace(re, QString("background-color: %1;").arg(color.name()));
    setStyleSheet(m_labelStyle);
}

void CStaticDisplay::setBackground(const QString& imageUrl)
{
    QString url = "url(" + imageUrl + ");";
    QRegularExpression re("background-image:.+?;");
    QRegularExpressionMatch match = re.match(m_style);

    if(match.hasMatch())
        m_style.replace(re, QString("background-image: %1").arg(url));
    else
        m_style += "background-repeat: no-repeat; background-position: center; background-image: " + url;

    setStyleSheet(m_style);
}

void CStaticDisplay::initLayout(const QString &text)
{
    QString bckColorName = qApp->palette().alternateBase().color().name();
    m_style = QString("background-color: %1; border-radius: 5px;").arg(bckColorName);
    m_labelStyle = QString("font-size: 12px; font-weight: bold; font-style: normal;");

    m_pLabel = new QLabel(text);
    m_pLabel->setAlignment(Qt::AlignCenter);
    m_pLabel->setAttribute(Qt::WA_TranslucentBackground);
    //Set font properties (size) with style sheet to overwrite application settings
    m_pLabel->setStyleSheet(m_labelStyle);
    m_pLayout->addWidget(m_pLabel);

    setStyleSheet(m_style);
}
