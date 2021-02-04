#include "CGraphicsRectPropWidget.h"
#include "Graphics/CGraphicsRectangle.h"
#include "Main/CoreTools.hpp"

CGraphicsRectPropWidget::CGraphicsRectPropWidget(QWidget *parent) : QWidget(parent)
{
    initLayout();
    initConnections();
}

void CGraphicsRectPropWidget::setProperties(GraphicsRectProperty *pProp)
{
    m_pProperty = pProp;
    updateUI();
}

void CGraphicsRectPropWidget::propertyChanged()
{
    updateUI();
}

void CGraphicsRectPropWidget::initLayout()
{
    QLabel* pLabelPenColor = new QLabel(tr("Line color"));
    m_pPenColorBtn = new CColorPushButton(tr("Line color"), QColor(255,0,0));

    QLabel* pLabelSize = new QLabel(tr("Line size"));
    m_pSpinSize = new QSpinBox();
    m_pSpinSize->setRange(1, 5);
    m_pSpinSize->setValue(1);

    QLabel* pLabelBrushColor = new QLabel(tr("Fill color"));
    m_pBrushColorBtn = new CColorPushButton(tr("Fill color"), QColor(255,0,0,0));

    QLabel* pLabelCategory = new QLabel(tr("Category"));
    m_pEditCategory = new QLineEdit("Default");

    QLabel* pLabelOpacity = new QLabel(tr("Opacity(%)"));
    m_pSpinOpacity = new QSpinBox();
    m_pSpinOpacity->setRange(0, 100);
    m_pSpinOpacity->setValue(100);

    QWidget* pEmpty = new QWidget();
    pEmpty->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    QGridLayout* pLayout = new QGridLayout();
    pLayout->addWidget(pLabelPenColor, 0, 0);
    pLayout->addWidget(m_pPenColorBtn, 0, 1);
    pLayout->addWidget(pLabelSize, 1, 0);
    pLayout->addWidget(m_pSpinSize, 1, 1);
    pLayout->addWidget(pLabelBrushColor, 2, 0);
    pLayout->addWidget(m_pBrushColorBtn, 2, 1);
    pLayout->addWidget(pLabelCategory, 3, 0);
    pLayout->addWidget(m_pEditCategory, 3, 1);
    pLayout->addWidget(pLabelOpacity, 4, 0);
    pLayout->addWidget(m_pSpinOpacity, 4, 1);
    pLayout->addWidget(pEmpty, 5, 0);

    setLayout(pLayout);
}

void CGraphicsRectPropWidget::initConnections()
{
    connect(m_pPenColorBtn, &CColorPushButton::colorChanged, [&](const QColor& color)
    {
        m_pProperty->m_penColor = Utils::Graphics::toCColor(color);
    });
    connect(m_pBrushColorBtn, &CColorPushButton::colorChanged, [&](const QColor& color)
    {
        m_pProperty->m_brushColor = Utils::Graphics::toCColor(color);
        m_pProperty->m_brushColor[3] = 255*m_pSpinOpacity->value()/100;
    });
    connect(m_pSpinSize, QOverload<int>::of(&QSpinBox::valueChanged), [&](int value)
    {
        m_pProperty->m_lineSize = value;
    });
    connect(m_pEditCategory, &QLineEdit::editingFinished, [&]
    {
        m_pProperty->m_category = m_pEditCategory->text().toStdString();
    });
    connect(m_pSpinOpacity, QOverload<int>::of(&QSpinBox::valueChanged), [&](int value)
    {
        m_pProperty->m_brushColor[3] = 255*value/100;
    });
}

void CGraphicsRectPropWidget::updateUI()
{
    if(m_pProperty)
    {
        m_pPenColorBtn->setColor(Utils::Graphics::toQColor(m_pProperty->m_penColor));
        m_pBrushColorBtn->setColor(Utils::Graphics::toQColor(m_pProperty->m_brushColor));
        m_pSpinSize->setValue(m_pProperty->m_lineSize);
        m_pEditCategory->setText(QString::fromStdString(m_pProperty->m_category));
        m_pSpinOpacity->setValue(m_pProperty->m_brushColor[3]*100/255);
    }
}
