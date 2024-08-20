// Copyright (C) 2021 Ikomia SAS
// Contact: https://www.ikomia.com
//
// This file is part of the IkomiaStudio software.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/**
 * @file      CImageDisplay.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CImageDisplay
 *
 * @details   Details
 */

#include "CImageDisplay.h"
#include "CImageView.h"
#include <QEvent>
#include <QSlider>
#include <QSpinBox>
#include <QPushButton>
#include "CImageExportDlg.h"
#include "Workflow/CViewPropertyIO.h"

CImageDisplay::CImageDisplay(QWidget* pParent, QString name, int flags) : CDataDisplay(pParent, name, flags)
{
    initLayout();
    initConnections();
    m_typeId = DisplayType::IMAGE_DISPLAY;
}

void CImageDisplay::initLayout()
{
    setObjectName("CImageDisplay");
    m_pView = new CImageView;

    int index = 0;
    if(m_flags & CHECKBOX)
        index++;
    if(m_flags & TITLE)
        index++;

    m_pHbox->insertStretch(index++, 1);

    if(m_flags & ZOOM_BUTTONS)
    {
        m_pZoomFitBtn = createButton(QIcon(":/Images/zoom-fit-best.png"), tr("Fit in view"));
        m_pZoomInBtn = createButton(QIcon(":/Images/zoom-in.png"), tr("Zoom in"));
        m_pZoomOutBtn = createButton(QIcon(":/Images/zoom-out.png"), tr("Zoom out"));
        m_pZoomOriginalBtn = createButton(QIcon(":/Images/zoom-original.png"), tr("Original size"));

        m_pHbox->insertSpacing(index++, m_toolbarSpacing);
        m_pHbox->insertWidget(index++, m_pZoomOriginalBtn);
        m_pHbox->insertWidget(index++, m_pZoomFitBtn);
        m_pHbox->insertWidget(index++, m_pZoomInBtn);
        m_pHbox->insertWidget(index++, m_pZoomOutBtn);
    }

    if(m_flags & GRAPHICS_BUTTON)
    {
        m_pHbox->insertSpacing(index++, m_toolbarSpacing);
        m_pGraphicsBtn = createButton(QIcon(":/Images/draw-freehand.png"), tr("Toggle graphics toolbar"));
        m_pHbox->insertWidget(index++, m_pGraphicsBtn);
    }

    if(m_flags & SAVE_BUTTON || m_flags & EXPORT_BUTTON)
    {
        m_pHbox->insertSpacing(index++, m_toolbarSpacing);
        if(m_flags & SAVE_BUTTON)
        {
            m_pSaveBtn = createButton(QIcon(":/Images/save.png"), tr("Save image"));
            m_pHbox->insertWidget(index++, m_pSaveBtn);
        }

        if(m_flags & EXPORT_BUTTON)
        {
            m_pExportBtn = createButton(QIcon(":/Images/export.png"), tr("Export image"));
            m_pHbox->insertWidget(index++, m_pExportBtn);
        }
    }

    if(m_flags & SLIDER)
    {
        m_pSlider = new QSlider(Qt::Horizontal);
        m_pSpinBox = new QSpinBox;
        m_pHbox->insertSpacing(index++, m_toolbarSpacing);
        m_pHbox->insertWidget(index++, m_pSlider);
        m_pHbox->insertWidget(index++, m_pSpinBox);
    }

    if(!(m_flags & MAXIMIZE_BUTTON))
        m_pHbox->insertStretch(index++, 1);

    m_pLayout->addWidget(m_pView);
}

void CImageDisplay::showMenu(const QPoint &pos)
{
    QMenu menu(this);

    QAction saveAction(QIcon(":Images/save.png"), tr("Save image"), this);
    connect(&saveAction, &QAction::triggered, [&]{ emit doSave(); });
    menu.addAction(&saveAction);

    QAction exportAction(QIcon(":Images/export.png"), tr("Export image"), this);
    connect(&exportAction, &QAction::triggered, this, &CImageDisplay::onExportBtnClicked);
    menu.addAction(&exportAction);

    menu.exec(mapToGlobal(pos));
}

void CImageDisplay::initConnections()
{
    if(m_flags & SLIDER)
    {
        connect(m_pSlider, &QSlider::valueChanged, this, &CImageDisplay::onSliderChanged);
        connect(m_pSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), m_pSlider, &QSlider::setValue);
    }

    if(m_flags & ZOOM_BUTTONS)
    {
        connect(m_pZoomFitBtn, &QPushButton::clicked, this, &CImageDisplay::onZoomFit);
        connect(m_pZoomInBtn, &QPushButton::clicked, this, &CImageDisplay::onZoomIn);
        connect(m_pZoomOutBtn, &QPushButton::clicked, this, &CImageDisplay::onZoomOut);
        connect(m_pZoomOriginalBtn, &QPushButton::clicked, this, &CImageDisplay::onZoomOriginal);
    }

    if(m_flags & SAVE_BUTTON)
        connect(m_pSaveBtn, &QPushButton::clicked, [&]{ emit doSave(); });

    if(m_flags & EXPORT_BUTTON)
        connect(m_pExportBtn, &QPushButton::clicked, this, &CImageDisplay::onExportBtnClicked);

    if(m_flags & GRAPHICS_BUTTON)
        connect(m_pGraphicsBtn, &QPushButton::clicked, [&]{ emit doToggleGraphicsToolbar(); });

    connect(this, &CImageDisplay::doSelect, [&](CDataDisplay* pDisplay)
    {
        Q_UNUSED(pDisplay);
        setSelected(true);
    });
}

void CImageDisplay::setImage(const QImage &image, bool bZoomFit)
{
    if(image.isNull())
        return;

    m_pView->setImage(image, bZoomFit);
}

void CImageDisplay::setImage(CImageScene* pScene, const QImage& image, bool bZoomFit)
{
    if(image.isNull())
        return;

    if(pScene)
        m_pView->setImageScene(pScene);

    m_pView->setImage(image, bZoomFit);
}

void CImageDisplay::setToolbarSpacing(int size)
{
    m_toolbarSpacing = size;
}

void CImageDisplay::setSliderRange(size_t max)
{
    if (m_pSlider && m_pSpinBox)
    {
        m_pSlider->setRange(0, (int)max-1);
        m_pSpinBox->setRange(0, (int)max-1);
        m_pSpinBox->adjustSize();
    }
}

void CImageDisplay::setCurrentImgIndex(size_t index)
{
    if (m_pSlider)
        m_pSlider->setValue((int)index);
}

void CImageDisplay::setViewSpaceShared(bool bShared)
{
    Q_UNUSED(bShared);

    //Keep zoom fit property
    if(m_pView->isZoomFit())
        m_pView->zoomFit();
}

void CImageDisplay::setSelected(bool bSelect)
{
    if(bSelect)
    {
        QString css = QString("CImageDisplay { border: 2px solid %1; }").arg(qApp->palette().highlight().color().name());
        setStyleSheet(css);
    }
    else
    {
        QString css = QString("CImageDisplay { border: 2px solid %1; }").arg(qApp->palette().base().color().name());
        setStyleSheet(css);
    }
}

CImageView*CImageDisplay::getView() const
{
    return m_pView;
}

void CImageDisplay::fitInView()
{
    m_pView->fitInView(m_pView->sceneRect(), Qt::KeepAspectRatio);
}

void CImageDisplay::applyViewProperty()
{
    CDataDisplay::applyViewProperty();
    applyZoomViewProperty();
}

void CImageDisplay::applyZoomViewProperty()
{
    if(m_pViewProperty == nullptr)
        return;

    m_pView->setNumScheduledScalings(m_pViewProperty->getZoomNumScheduledScalings());
    m_pView->setTargetScenePos(m_pViewProperty->getZoomTargetPos());

    if(m_pViewProperty->isZoomFit())
        m_pView->zoomFit();
    else
    {
        m_pView->setScale(m_pViewProperty->getZoomScale());
        // Use singleShot to ensure centering at first show
        // Use onUpdateCenter() to avoid emitting scrollbar changed
        QTimer::singleShot(0, this, [&]
        {
            if(m_pView && m_pViewProperty)
                m_pView->onUpdateCenter(m_pViewProperty->getViewCenter());
        });
    }
}

void CImageDisplay::enableImageMoveByKey(bool bEnable)
{
    m_pView->enableMoveByKey(bEnable);
}

void CImageDisplay::enableGraphicsInteraction(bool bEnable)
{
    if(m_pView)
        m_pView->setInteractive(bEnable);
}

bool CImageDisplay::eventFilter(QObject* obj, QEvent* event)
{
    if(obj == this)
    {
        if(event->type() == QEvent::MouseButtonDblClick)
        {
            emit doDoubleClicked(this);
            return true;
        }
        else if(event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(event);
            if(pMouseEvent->button() & Qt::RightButton && (m_flags & SAVE_BUTTON) && (m_flags & EXPORT_BUTTON))
            {
                showMenu(pMouseEvent->pos());
                return true;
            }
        }
    }

    return CDataDisplay::eventFilter(obj, event);
}

void CImageDisplay::resizeEvent(QResizeEvent *event)
{
    CDataDisplay::resizeEvent(event);

    if(m_pViewProperty != nullptr && m_pViewProperty->isZoomFit())
        m_pView->zoomFit();
}

void CImageDisplay::keyPressEvent(QKeyEvent *event)
{
    if(!m_bMoveByKey && (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
                         event->key() == Qt::Key_Left || event->key() == Qt::Key_Right))
    {
        event->ignore();
    }
    else
        CDataDisplay::keyPressEvent(event);
}

void CImageDisplay::onUpdateIndex(int index)
{
    m_pSlider->setValue(index);
}

void CImageDisplay::onSliderChanged(int index)
{
    if(index != m_pSpinBox->value())
        m_pSpinBox->setValue(index);

    emit doUpdateIndex(index);
}

void CImageDisplay::onZoomFit()
{
    m_pView->onZoomFitEvent();
}

void CImageDisplay::onZoomIn()
{
    m_pView->onDispatchZoomIn();
}

void CImageDisplay::onZoomOut()
{
    m_pView->onDispatchZoomOut();
}

void CImageDisplay::onZoomOriginal()
{
    m_pView->onZoomOriginalEvent();
}

void CImageDisplay::onExportBtnClicked()
{
    CImageExportDlg exportDlg(tr("Export image"), CImageExportDlg::IMAGE, this);
    if(exportDlg.exec() == QDialog::Accepted)
        emit doExport(exportDlg.getFileName(), exportDlg.isGraphicsExported());
}

#include "moc_CImageDisplay.cpp"

