#ifndef CHTMLDELEGATE_H
#define CHTMLDELEGATE_H

#include <QStyledItemDelegate>
#include <mutex>

class CHtmlDelegate : public QStyledItemDelegate
{
    public:
        void setMutex(std::mutex* pMutex);

    protected:
        void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
        QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

        static int m_padding;

    private:
        mutable std::mutex* m_pMutex = nullptr;
};


#endif // CHTMLDELEGATE_H
