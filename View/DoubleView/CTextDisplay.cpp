#include "CTextDisplay.h"
#include "Main/AppTools.hpp"

CTextDisplay::CTextDisplay(const QString &name, QWidget *parent, int flags) : CDataDisplay(parent, name, flags)
{
    m_typeId = DisplayType::TEXT_DISPLAY;
    initLayout();
    initConnections();
}

CTextDisplay::CTextDisplay(const QString &name, const QString &text, QWidget *parent, int flags) : CDataDisplay(parent, name, flags)
{
    m_typeId = DisplayType::TEXT_DISPLAY;
    initLayout(text);
    initConnections();
}

void CTextDisplay::initLayout(const QString& text)
{
    setObjectName("CTextDisplay");

    m_pExportBtn = createButton(QIcon(":/Images/export.png"));

    int index = 0;
    if(m_flags & CHECKBOX)
        index++;
    if(m_flags & TITLE)
        index++;

    if(m_flags & EXPORT_BUTTON)
    {
        m_pHbox->insertStretch(index++, 1);
        m_pHbox->insertWidget(index++, m_pExportBtn);
        m_pHbox->insertStretch(index++, 1);
    }

    m_pTextEdit = new QTextEdit();
    m_pTextEdit->setReadOnly(true);
    m_pTextEdit->setPlainText(text);
    m_pLayout->addWidget(m_pTextEdit);
}

void CTextDisplay::initConnections()
{
    connect(m_pExportBtn, &QPushButton::clicked, this, &CTextDisplay::onExportBtnClicked);
}

QPushButton *CTextDisplay::createButton(const QIcon &icon)
{
    auto pal = qApp->palette();
    auto color = pal.highlight().color();

    auto pBtn = new QPushButton;
    pBtn->setIcon(icon);
    pBtn->setIconSize(QSize(16,16));
    pBtn->setFixedSize(22,22);
    pBtn->setStyleSheet(QString("QPushButton { background: transparent; border: none;} QPushButton:hover {border: 1px solid %1;}").arg(color.name()));
    return pBtn;
}

void CTextDisplay::onExportBtnClicked()
{
    QString path = Utils::File::saveFile(this, tr("Export text file"), "", tr("Text file (*.txt)"), QStringList({"txt"}), ".txt");
    QFile file(path);

    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << m_pTextEdit->toPlainText();
    }
}

void CTextDisplay::setText(const QString &text)
{
    m_pTextEdit->setPlainText(text);
}

bool CTextDisplay::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == this)
    {
        if(event->type() == QEvent::MouseButtonDblClick)
        {
            emit doDoubleClicked(this);
            return true;
        }
    }
    return CDataDisplay::eventFilter(obj, event);
}


