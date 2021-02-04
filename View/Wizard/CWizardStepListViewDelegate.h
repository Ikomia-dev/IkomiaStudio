#ifndef CWIZARDSTEPLISTVIEWDELEGATE_H
#define CWIZARDSTEPLISTVIEWDELEGATE_H

#include <QStyledItemDelegate>

class CWizardStepListViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:

        CWizardStepListViewDelegate(QObject* parent = nullptr);

    protected:

        void    paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

        QSize   sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

    private:

        void    paintBorders(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex & index) const;
        void    paintInfo(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    private:

        QSize   m_size = QSize(INT_MAX, 32);
};

#endif // CWIZARDSTEPLISTVIEWDELEGATE_H
