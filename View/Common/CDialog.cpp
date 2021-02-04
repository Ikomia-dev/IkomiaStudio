#include "CDialog.h"
#include <QtWidgets>

CDialog::CDialog(QWidget *parent, int style, Qt::WindowFlags f) :
    QDialog(parent, f | Qt::FramelessWindowHint)
{
    m_styleFlags = style;
    setAttribute(Qt::WA_TranslucentBackground);
    initLayout();
    initFramelessHelper();

    if(style & EFFECT_ENABLED)
        applyEffects();
}

CDialog::CDialog(const QString title, QWidget *parent, int style, Qt::WindowFlags f) :
    QDialog(parent, f | Qt::FramelessWindowHint)
{
    m_title = title;
    m_styleFlags = style;
    setAttribute(Qt::WA_TranslucentBackground);
    initLayout();
    initFramelessHelper();

    if(style & EFFECT_ENABLED)
        applyEffects();
}

void CDialog::setTitle(const QString &title)
{
    m_title = title;
    m_pLabelTitle->setText(title);
}

QVBoxLayout *CDialog::getContentLayout() const
{
    return m_pContentLayout;
}

int CDialog::getBorderSize() const
{
    auto sizeFrame = m_pFrame->size();
    auto sizeDialog = this->size();
    return (sizeDialog.width() - sizeFrame.width())/2;
}

void CDialog::initLayout()
{
    m_pContentLayout = new QVBoxLayout;
    m_pContentLayout->setContentsMargins(5, 5, 5, 5);

    if(m_styleFlags & NO_TITLE_BAR)
    {
        m_pFrame = new QFrame;
        m_pFrame->setLayout(m_pContentLayout);
        m_pFrame->setProperty("class", "CDialogStyle");
    }
    else
    {
        initTitleBar();

        auto pGlobalLayout = new QVBoxLayout;
        pGlobalLayout->setContentsMargins(0,0,0,0);
        pGlobalLayout->addWidget(m_pTitleBar);
        pGlobalLayout->addLayout(m_pContentLayout);

        m_pFrame = new QFrame;
        m_pFrame->setLayout(pGlobalLayout);
        m_pFrame->setProperty("class", "CDialogStyle");
    }

    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->setContentsMargins(0,0,0,0);
    pMainLayout->addWidget(m_pFrame);

    setLayout(pMainLayout);
}

void CDialog::initTitleBar()
{
    QHBoxLayout* pLayout = new QHBoxLayout;
    pLayout->setContentsMargins(5, 1, 5, 1);

    if(m_styleFlags & LABEL)
    {
        m_pLabelTitle = new QLabel(m_title);
        m_pLabelTitle->setStyleSheet("background: transparent; border: transparent;");
        pLayout->addWidget(m_pLabelTitle);
    }

    pLayout->addStretch(1);

    if(m_styleFlags & MAXIMIZE_BUTTON)
    {
        QPushButton* pBtnMaximize = createTitleButton(":Images/view-fullscreen.png");
        connect(pBtnMaximize, &QPushButton::clicked, [this]
        {
            if(m_bIsMaximized)
            {
                showNormal();
                adjustSize();
                m_bIsMaximized = false;
            }
            else
            {
                showMaximized();
                m_bIsMaximized = true;
            }
        });
        pLayout->addWidget(pBtnMaximize);
    }

    if(m_styleFlags & CLOSE_BUTTON)
    {
        QPushButton* pBtnClose = createTitleButton(":Images/closeApp.png");
        connect(pBtnClose, &QPushButton::clicked, this, &CDialog::close);
        pLayout->addWidget(pBtnClose);
    }

    m_pTitleBar = new QFrame;
    m_pTitleBar->setFixedHeight(m_titleBarHeight);
    m_pTitleBar->setProperty("class", "CHeaderDialogStyle");
    m_pTitleBar->setLayout(pLayout);
}

void CDialog::initFramelessHelper()
{
    if(m_styleFlags & RESIZABLE || m_styleFlags & MOVABLE)
    {
        m_pViewMover = std::make_unique<CFramelessHelper>();
        m_pViewMover->activateOn(this);
        m_pViewMover->setWidgetMovable(m_styleFlags & MOVABLE);
        m_pViewMover->setWidgetResizable(m_styleFlags & RESIZABLE);
        m_pViewMover->setBorderWidth(m_titleBarHeight);
        m_pViewMover->setWidgetGeometry(m_pFrame);
    }
}

void CDialog::applyEffects()
{
    //Drop shadow effect
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
    effect->setBlurRadius(5);
    effect->setColor(qApp->palette().base().color().darker());
    setGraphicsEffect(effect);
}

QPushButton *CDialog::createTitleButton(const QString iconPath)
{
    QPushButton* pBtn = new QPushButton(QIcon(iconPath), "");
    pBtn->setStyleSheet(QString("QPushButton { background: transparent; border: none;} QPushButton:hover {border: 1px solid %1;}")
                        .arg(qApp->palette().highlight().color().name()));
    pBtn->setFlat(true);
    pBtn->setFixedSize(QSize(18, 18));
    pBtn->setIconSize(QSize(16, 16));
    return pBtn;
}

void CDialog::showCustomMaximized()
{
    QScreen* pScreen = this->window()->windowHandle()->screen();
    QSize desktopSize = pScreen->availableSize();
    resize(desktopSize);
    m_bIsMaximized = true;
}
