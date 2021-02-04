#ifndef CWIZARDTUTOLISTVIEWDELEGATE_H
#define CWIZARDTUTOLISTVIEWDELEGATE_H

#include "View/Common/CListViewDelegate.h"

class CWizardTutoListViewDelegate : public CListViewDelegate
{
    Q_OBJECT

    public:

        enum Actions { NONE, INFO, PLAY };

        CWizardTutoListViewDelegate(QObject* parent = nullptr);

    signals:

        void            doShowInfo(const QModelIndex& index);
        void            doPlayTuto(const QModelIndex& index);

    protected:

        void            paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

        virtual QString getActionIconPath(int action) const override;
        virtual int     getBtnAction(int index) const override;

        virtual bool    isBtnEnabled(const QModelIndex& itemIndex, int index) const override;

        virtual void    executeAction(int action, const QModelIndex& index) override;

        virtual void    showTooltip(const QModelIndex& modelIndex, const QPoint& pos, int index) const override;

    private:

        virtual void    initBrush() override;

        void            paintBorders(QPainter* painter, const QStyleOptionViewItem& option) const;
        void            paintIcon(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        void            paintTutoInfo(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        void            paintActionButtons(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex &index) const;
        void            paintButton(QPainter* painter, const QRect& rc, const QString &pixmapPath, bool bHovered) const;

    private:

        const int               m_actionBtnCount = 2;
};

#endif // CWIZARDTUTOLISTVIEWDELEGATE_H
