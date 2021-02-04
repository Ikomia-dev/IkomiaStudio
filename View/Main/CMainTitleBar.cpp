/**
 * @file      CMainTitleBar.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CMainTitleBar
 *
 * @details   Details
 */

#include "CMainTitleBar.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
#include "Main/AppTools.hpp"

CMainTitleBar::CMainTitleBar(int size, QWidget* parent) : QFrame(parent)
{
    m_size = size;
    m_pLayout = new QHBoxLayout;
    m_pLayout->setMargin(0);
    m_pLayout->setSpacing(0);

    QVBoxLayout* pVbox = new QVBoxLayout;
    pVbox->setContentsMargins(5,5,5,5);
    pVbox->addLayout(m_pLayout);

    setLayout(pVbox);
    setStyleSheet("background: rgb(40,40,40);");

    initTitle();
    initButtons();
    initConnections();
}

void CMainTitleBar::setUser(const QString &name)
{
    Q_UNUSED(name);
    /*QString user = "";
    if(name.isEmpty() == false)
        user = QString(" (%1 - %2)").arg(name).arg(Utils::User::getUserRoleName(role));

    if(name.isEmpty())
        m_pLabelTitle->setText(QApplication::applicationName());
    else
        m_pLabelTitle->setText(QApplication::applicationName() + " - " + name);*/
}

void CMainTitleBar::initTitle()
{
    assert(this->parentWidget() != nullptr);

    QLabel* pIcon = new QLabel;
    pIcon->setPixmap(QPixmap(":Images/logo_app.png"));

    m_pLabelTitle = new QLabel();
    /*m_pLabelTitle = new QLabel(QApplication::applicationName());
    QFont font = m_pLabelTitle->font();
    font.setBold(true);
    font.setCapitalization(QFont::SmallCaps);
    m_pLabelTitle->setFont(font);
    m_pLabelTitle->setStyleSheet("background: transparent;");*/
    //m_pLabelTitle->setStyleSheet("background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #323232, stop: 0.5 #101010, stop: 1.0 #323232);");

    m_pLayout->addWidget(pIcon, 0, Qt::AlignLeft);
    m_pLayout->addWidget(m_pLabelTitle, 0, Qt::AlignLeft);
    m_pLayout->addStretch(1);
}

void CMainTitleBar::initButtons()
{
    QIcon closeIcon(":Images/closeApp.png");
    QIcon minimizeIcon(":Images/minimize.png");
    QIcon expandIcon(":Images/view-fullscreen.png");

    auto pal = qApp->palette();
    auto color = pal.highlight().color();

    m_pClose = new QPushButton(closeIcon, "");
    m_pClose->setStyleSheet(QString("QPushButton { background: transparent; border: none;} QPushButton:hover {border: 1px solid %1;}").arg(color.name()));
    m_pClose->setFlat(true);
    m_pClose->setFixedSize(QSize(m_size, m_size));
    m_pClose->setIconSize(QSize(m_size, m_size));

    m_pMinimize = new QPushButton(minimizeIcon, "");
    m_pMinimize->setStyleSheet(QString("QPushButton { background: transparent; border: none;} QPushButton:hover {border: 1px solid %1;}").arg(color.name()));
    m_pMinimize->setFlat(true);
    m_pMinimize->setFixedSize(QSize(m_size, m_size));
    m_pMinimize->setIconSize(QSize(m_size, m_size));

    m_pExpand = new QPushButton(expandIcon, "");
    m_pExpand->setStyleSheet(QString("QPushButton { background: transparent; border: none;} QPushButton:hover {border: 1px solid %1;}").arg(color.name()));
    m_pExpand->setFlat(true);
    m_pExpand->setFixedSize(QSize(m_size, m_size));
    m_pExpand->setIconSize(QSize(m_size, m_size));

    m_pLayout->setSpacing(2);
    m_pLayout->addWidget(m_pMinimize);
    m_pLayout->addWidget(m_pExpand);
    m_pLayout->addWidget(m_pClose);
}

void CMainTitleBar::initConnections()
{
    assert(m_pClose != nullptr);
    assert(m_pMinimize != nullptr);
    assert(m_pExpand != nullptr);

    connect(m_pClose, &QPushButton::clicked, this, &CMainTitleBar::close);
    connect(m_pMinimize, &QPushButton::clicked, this, &CMainTitleBar::minimize);
    connect(m_pExpand, &QPushButton::clicked, this, &CMainTitleBar::expand);
}

#include "moc_CMainTitleBar.cpp"
