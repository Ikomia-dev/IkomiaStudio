#ifndef CPROCESSLISTVIEWDELEGATE_H
#define CPROCESSLISTVIEWDELEGATE_H

#include "View/Common/CListViewDelegate.h"

/**
 * @brief
 *
 */
class CProcessListViewDelegate : public CListViewDelegate
{
    Q_OBJECT

    public:

        enum Actions { NONE, ADD, INFO };

        CProcessListViewDelegate(QObject* parent = nullptr);

    signals:

        void            doShowInfo(const QModelIndex& index);
        void            doAddProcess(const QString& processName);

    protected:

        virtual int     getBtnAction(int index) const override;
        virtual QString getActionIconPath(int action) const override;

        virtual bool    isBtnEnabled(const QModelIndex& itemIndex, int index) const override;

        virtual void    executeAction(int action, const QModelIndex& index) override;

        virtual void    showTooltip(const QModelIndex& modelIndex, const QPoint& pos, int index) const override;

    private:

        void            paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
        virtual void    paintText(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex& index) const;
        virtual void    paintLanguageIcon(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
        virtual void    paintOSIcon(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // CPROCESSLISTVIEWDELEGATE_H
