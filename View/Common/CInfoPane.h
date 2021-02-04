#ifndef CINFOPANE_H
#define CINFOPANE_H

#include <QWidget>
#include <QToolBox>
#include <Widgets/QtPropertyBrowser/qttreepropertybrowser.h>
#include <Widgets/QtPropertyBrowser/qtbuttonpropertybrowser.h>
#include <Widgets/QtPropertyBrowser/qtvariantproperty.h>

#include "Main/AppDefine.hpp"

class CInfoPane : public QWidget
{
    Q_OBJECT

    public:

        CInfoPane();

    signals:

        void    doEnableInfo(bool bEnable);

    public slots:

        void    onDisplayImageInfo(const VectorPairString &infoList);
        void    onDisplayVideoInfo(const VectorPairString& infoList);

    protected:

        void    showEvent(QShowEvent* event);
        void    hideEvent(QHideEvent* event);

    private:

        void    initLayout();

        void    fillProperties(const VectorPairString &infoList, const QString &title);
        void    fillPropertySize(QtProperty* pItem);

    private:

        QToolBox*                   m_pInfoToolBox = nullptr;
        QtTreePropertyBrowser*      m_pPropertyBrowser = nullptr;
        QtVariantPropertyManager*   m_pVariantManager = nullptr;
        VectorPairString            m_infoList;
};

#endif // CINFOPANE_H
