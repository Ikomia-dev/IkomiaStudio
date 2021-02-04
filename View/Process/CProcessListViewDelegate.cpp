#include "CProcessListViewDelegate.h"
#include "Model/Project/CMultiProjectModel.h"
#include "Model/Process/CProcessManager.h"
#include <QFileSystemModel>
#include <QStaticText>
#include <QPainter>
#include <QStylePainter>

CProcessListViewDelegate::CProcessListViewDelegate(QObject* parent) : CListViewDelegate(parent)
{
    m_btnSize = QSize(32, 32);
}

QString CProcessListViewDelegate::getActionIconPath(int action) const
{
    QString path;
    switch(action)
    {
        case NONE: break;
        case INFO: path = ":/Images/info-color2.png"; break;
        case ADD: path = ":/Images/add-input.png"; break;
    }
    return path;
}

bool CProcessListViewDelegate::isBtnEnabled(const QModelIndex &itemIndex, int index) const
{
    Q_UNUSED(itemIndex);
    Q_UNUSED(index);
    return true;
}

void CProcessListViewDelegate::executeAction(int action, const QModelIndex &index)
{
    switch(action)
    {
        case NONE: break;
        case INFO: emit doShowInfo(index); break;
        case ADD: emit doAddProcess(index.data().toString()); break;
    }
}

void CProcessListViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Main bounding rect
    paintBoundingRect(painter, option);
    // Process icon
    paintIcon(painter, option, index);
    // Name + description
    paintText(painter, option, index);
    // Language icon
    paintLanguageIcon(painter, option, index);
    // Actions button (Info, Add)
    paintActionButtons(painter, option, index);
}

void CProcessListViewDelegate::paintText(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    QBrush brushHighlight = option.palette.highlight();
    QColor colorHighlight = brushHighlight.color();
    QBrush brushText = option.palette.text();
    QColor colorText = brushText.color();

    QFont fontNormal = qApp->font();
    fontNormal.setPointSize(11);
    QFont fontBold = qApp->font();
    fontBold.setBold(true);
    fontBold.setPointSize(12);

    //Process name
    QRect rcName(option.rect.left() + m_contentMargins.left() + m_spacings.width() + m_headerHeight,
                 option.rect.top() + m_contentMargins.top(),
                 option.rect.width() - m_contentMargins.left() - m_spacings.width() - m_contentMargins.right() - m_headerHeight,
                 m_headerHeight);

    QFontMetrics fontMetricsBold(fontBold);
    QString name = getDataName(index);
    QString displayName = fontMetricsBold.elidedText(name, Qt::TextElideMode::ElideRight, rcName.width());
    auto w = fontMetricsBold.horizontalAdvance(displayName);
    paintStaticText(painter, rcName.center().x() - w/2, rcName.top(), rcName.width(), displayName, fontBold, colorHighlight);

    //Process description
    int bodyTop = rcName.bottom() + m_contentMargins.top() * 3;
    int bodyLeft = option.rect.left() + m_iconSize.width() + m_contentMargins.left() + m_spacings.width();
    int bodyWidth = option.rect.width() - m_iconSize.width() - m_contentMargins.left() - m_spacings.width() - m_contentMargins.right();

    QString description = getShortDescription(index);
    if(description.isEmpty())
        description = getDescription(index);

    QString displayDescription = Utils::String::getElidedString(description, fontNormal, bodyWidth, 3);
    paintStaticText(painter, bodyLeft, bodyTop, bodyWidth, displayDescription, fontNormal, colorText);
}

int CProcessListViewDelegate::getBtnAction(int index) const
{
    int action = NONE;
    switch(index)
    {
        case 0: action = INFO; break;
        case 1: action = ADD; break;
    }
    return action;
}

void CProcessListViewDelegate::paintLanguageIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    assert(index.isValid());
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QRect rc(   option.rect.left() + m_contentMargins.left() + m_spacings.width(),
                option.rect.top() + m_contentMargins.top(),
                m_headerHeight,
                m_headerHeight);

    auto pModel = static_cast<const CProcessTableProxyModel*>(index.model());
    auto srcIndex = pModel->mapToSource(index);
    auto pTable = static_cast<const QSqlTableModel*>(srcIndex.model());
    int language = pTable->record(index.row()).value("language").toInt();

    QString pixmapPath;
    if(language == CProcessInfo::CPP)
        pixmapPath = ":/Images/C++-language-logo.png";
    else
        pixmapPath = ":/Images/python-language-logo.png";

    QPixmap pixmap = QPixmap(pixmapPath).scaled(m_headerHeight, m_headerHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter->drawPixmap(rc, pixmap);
    painter->restore();
}

void CProcessListViewDelegate::paintOSIcon(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    assert(index.isValid());
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QRect rc(   option.rect.left() + m_contentMargins.left() + m_spacings.width(),
                option.rect.bottom() - m_contentMargins.bottom() - m_btnSize.height(),
                m_btnSize.width(),
                m_btnSize.height());

    auto pModel = static_cast<const CProcessTableProxyModel*>(index.model());
    auto srcIndex = pModel->mapToSource(index);
    auto pTable = static_cast<const QSqlTableModel*>(srcIndex.model());
    int os = pTable->record(index.row()).value("os").toInt();

    QString pixmapPath;
    if(os == CProcessInfo::LINUX)
        pixmapPath = ":/Images/linux.png";
    else if(os == CProcessInfo::WIN)
        pixmapPath = ":/Images/win.png";
    else if(os == CProcessInfo::OSX)
        pixmapPath = ":/Images/mac.png";
    else
        pixmapPath = ":/Images/all.png";

    QPixmap pixmap = QPixmap(pixmapPath).scaled(m_btnSize.width(), m_btnSize.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter->drawPixmap(rc, pixmap);
    painter->restore();
}

void CProcessListViewDelegate::showTooltip(const QModelIndex &modelIndex, const QPoint& pos, int index) const
{
    Q_UNUSED(modelIndex);
    switch(index)
    {
        case 0:
            QToolTip::showText(pos, tr("Info"));
            break;
        case 1:
            QToolTip::showText(pos, tr("Add"));
            break;
    }
}
