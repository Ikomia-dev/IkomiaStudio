#ifndef CROLLUPWIDGET_H
#define CROLLUPWIDGET_H

#include <QWidget>

class QPushButton;
class QWidget;
class QFrame;
class QHBoxLayout;
class CRollupBtn;

class CRollupWidget : public QWidget
{
    public:

        CRollupWidget(QWidget* parent = Q_NULLPTR);

        void        addWidget(const QIcon& icon, const QString& name, QWidget* pWidget);

        QFrame*     makeFrameWidget(QWidget* pWidget);

        void        insertWidget(int position, QString name, QWidget* pWidget);

        QWidget*    replaceWidget(int position, QString newName, QWidget* pWidget);

        void        removeWidget(QWidget *pWidget);
        QWidget*    removeWidget(int position);

        void        expand(int position);
        void        expand(QWidget* pWidget);
        void        expandAll();

        void        collapse(int position);
        void        collapse(QWidget* pWidget);
        void        collapseAll();

    private:

        QList<CRollupBtn*>  m_buttons;
        QList<QWidget*>     m_widgets;
        QWidget*            m_pContainer;
};

#endif // CROLLUPWIDGET_H
