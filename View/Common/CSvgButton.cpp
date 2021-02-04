#include "CSvgButton.h"
#include <QtSvg>

CSvgButton::CSvgButton(const QString& filePath, bool isCircular, QWidget* parent) : QToolButton(parent)
{
    m_filePath = filePath;
    m_bIsCircular = isCircular;
    initLayout();
}

CSvgButton::~CSvgButton()
{

}

void CSvgButton::setImage(const QString& filePath)
{
    m_filePath = filePath;
    m_pSvgWidget->load(m_filePath);
}

void CSvgButton::setCircular(bool bIsCircular)
{
    m_bIsCircular = bIsCircular;
}

void CSvgButton::initLayout()
{
    m_pSvgWidget = new QSvgWidget(m_filePath);
    m_pSvgWidget->setStyleSheet("background: transparent;");
    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->setContentsMargins(0,0,0,0);
    pLayout->addWidget(m_pSvgWidget);
    setLayout(pLayout);

    if(m_bIsCircular)
        this->setStyleSheet("QToolButton {"
                            "background: transparent;"
                            "border: 2px transparent red;"
                            "border-radius: 10px;"
                            "}");
}
