#ifndef CWIZARDTUTOLISTVIEW_H
#define CWIZARDTUTOLISTVIEW_H

#include <QListView>

class CWizardTutoListViewDelegate;

class CWizardTutoListView : public QListView
{
    Q_OBJECT

    public:

        CWizardTutoListView(QWidget* parent = nullptr);

    signals:

        void    doShowTutoInfo(const QModelIndex& index);
        void    doPlayTuto(const QModelIndex& index);

    protected:
        virtual void    mouseMoveEvent(QMouseEvent* event) override;

    private:

        void    initConnections();

    private:

        QSize                           m_itemSize = QSize(150, 120);
        CWizardTutoListViewDelegate*    m_pDelegate = nullptr;
};

#endif // CWIZARDTUTOLISTVIEW_H
