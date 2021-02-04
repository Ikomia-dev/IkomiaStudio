#ifndef CSTATICDISPLAY_H
#define CSTATICDISPLAY_H

#include "CDataDisplay.h"

class CStaticDisplay : public CDataDisplay
{
    public:

        CStaticDisplay(const QString& name, QWidget* parent = nullptr);
        CStaticDisplay(const QString& text, const QString& name, QWidget* parent = nullptr);

        void    setFont(int pointSize=12, int weight=QFont::Normal, bool bItalic=false);
        void    setBackgroundColor(QColor color);
        void    setBackground(const QString &imageUrl);

    private:

        void    initLayout(const QString& text="");

    private:

        QLabel* m_pLabel = nullptr;
        QString m_style;
        QString m_labelStyle;
};

#endif // CSTATICDISPLAY_H
