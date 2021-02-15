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
 * @file      C3dDisplay.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for C3dDisplay
 *
 * @details   Details
 */

#include "C3dDisplay.h"
#include <QPushButton>
#include <QComboBox>
#include <QAction>
#include <QLabel>
#include <QSlider>
#include <QFileDialog>
#include "C3dAnimationDlg.h"
#include "VolumeRenderDefine.h"
#include "Main/AppTools.hpp"

C3dDisplay::C3dDisplay(QWidget *parent, const QString &name) : CDataDisplay(parent, name)
{
    initLayout();
    initConnections();
    m_typeId = DisplayType::OPENGL_DISPLAY;
}

bool C3dDisplay::eventFilter(QObject* obj, QEvent* event)
{
    if(obj == this)
    {
        if(event->type() == QEvent::MouseButtonDblClick)
        {
            emit doDoubleClicked(this);
            return true;
        }
    }
    return CDataDisplay::eventFilter(obj, event);
}

void C3dDisplay::initLayout()
{
    setObjectName("C3dDisplay");
    m_pGLWidget = new CGLWidget;

    QWidget* pWidget = new QWidget;
    pWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_pVBoxLayout = new QVBoxLayout(pWidget);
    m_pVBoxLayout->setSpacing(0);
    m_pVBoxLayout->setContentsMargins(0,0,0,0);

    initToolBar();
    initSettings();

    m_pLayout->addWidget(pWidget);
    m_pLayout->addWidget(m_pGLWidget);
    m_pGLWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
}

void C3dDisplay::initConnections()
{
    connect(m_pColorMapBtn, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index){ emit doUpdateColormap(static_cast<size_t>(index));});

    //Connect Buttons
    connect(m_pRaycastBtn, &QPushButton::clicked, this, &C3dDisplay::onUseRaycasting);
    connect(m_pMIPBtn, &QPushButton::clicked, this, &C3dDisplay::onUseMIP);
    connect(m_pMinIPBtn, &QPushButton::clicked, this, &C3dDisplay::onUseMinIP);
    connect(m_pIsoBtn, &QPushButton::clicked, this, &C3dDisplay::onUseIso);
    connect(m_pScreenshotBtn, &QPushButton::clicked, this, &C3dDisplay::onExportScreenshot);
    connect(m_pAnimationBtn, &QPushButton::clicked, this, &C3dDisplay::onExportAnimation);

    //CGLWidget -> C3dDisplay
    connect(m_pGLWidget, &CGLWidget::doAddAnimationSequence, [&](const C3dAnimationSequence& sequence)
    {
        if(m_pAnimationDlg != nullptr)
            m_pAnimationDlg->addAnimationSequence(sequence);
    });
}

void C3dDisplay::initToolBar()
{
    m_pRaycastBtn = createToolbarBtn(tr("Raycasting"), QIcon(":/Images/sphere.png"));
    m_pMIPBtn = createToolbarBtn(tr("MIP"), QIcon(":/Images/sphere-MIP.png"));
    m_pMinIPBtn = createToolbarBtn(tr("MinIP"), QIcon(":/Images/sphere-MinIP.png"));
    m_pIsoBtn = createToolbarBtn(tr("Isosurface"), QIcon(":/Images/sphere-iso.png"));
    m_pSettingsBtn = createToolbarBtn(tr("Settings"), QIcon(":/Images/properties_white.png"));
    m_pScreenshotBtn = createToolbarBtn(tr("Screenshot"), QIcon(":/Images/view-preview.png"));
    m_pAnimationBtn = createToolbarBtn(tr("Animation"), QIcon(":/Images/view-video.png"));

    //QLabel* pLabel = new QLabel(tr("Colormap"));
    m_pColorMapBtn = new QComboBox;
    m_pColorMapBtn->addItem(QIcon(":/Images/grayscale.bmp"), tr("Classic"));
    m_pColorMapBtn->addItem(QIcon(":/Images/skin.bmp"), tr("Skin"));
    m_pColorMapBtn->addItem(QIcon(":/Images/jet.bmp"), tr("Jet"));
    m_pColorMapBtn->addItem(QIcon(":/Images/graycolor.bmp"), tr("Graycolor"));
    m_pColorMapBtn->setMinimumWidth(0);

    QHBoxLayout* pHBoxLayout = new QHBoxLayout;
    pHBoxLayout->setContentsMargins(0,0,0,0);
    pHBoxLayout->addWidget(m_pRaycastBtn);
    pHBoxLayout->addWidget(m_pMIPBtn);
    pHBoxLayout->addWidget(m_pMinIPBtn);
    pHBoxLayout->addWidget(m_pIsoBtn);
    pHBoxLayout->addStretch(1);
    pHBoxLayout->addWidget(m_pSettingsBtn);
    pHBoxLayout->addWidget(m_pScreenshotBtn);
    pHBoxLayout->addWidget(m_pAnimationBtn);
    pHBoxLayout->addStretch(1);
    //pHBoxLayout->addWidget(pLabel);
    pHBoxLayout->addWidget(m_pColorMapBtn);

    // Toolbar design
    m_pToolbar = new QWidget;
    m_pToolbar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    m_pToolbar->setLayout(pHBoxLayout);

    m_pVBoxLayout->addWidget(m_pToolbar);
}

QPushButton* C3dDisplay::createToolbarBtn(const QString& tooltip, const QIcon& icon)
{
    auto pal = qApp->palette();
    auto color = pal.light().color();
    auto pBtn = new QPushButton;
    pBtn->setMinimumWidth(0);
    pBtn->setFixedSize(22,22);
    pBtn->setIcon(icon);
    pBtn->setStyleSheet(QString("QPushButton { background: transparent; border: none;} QPushButton:hover {border: 1px solid %1;}").arg(color.name()));
    pBtn->setToolTip(tooltip);
    return pBtn;
}

void C3dDisplay::initSettings()
{
    m_pSettings = new QWidget;
    m_pSettings->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_pSettings->hide();

    QGridLayout* pSettingsLayout = new QGridLayout(m_pSettings);
    QSlider* pOffsetSlider = new QSlider(Qt::Horizontal);
    pOffsetSlider->setRange(0, 1000);

    connect(pOffsetSlider, &QSlider::valueChanged, this, &C3dDisplay::onUpdateOffset);

    pSettingsLayout->addWidget(new QLabel("Offset : "), 0, 0);
    pSettingsLayout->addWidget(pOffsetSlider, 0, 1);

    m_pVBoxLayout->addWidget(m_pSettings);

    connect(m_pSettingsBtn, &QPushButton::clicked, [&]{
        if(m_pSettings->isHidden())
            m_pSettings->show();
        else
            m_pSettings->hide();
    });
}

void C3dDisplay::initGL()
{
    if(m_pGLWidget)
        m_pGLWidget->initGL();
}

void C3dDisplay::changeVolume()
{
    m_pGLWidget->init();
}

void C3dDisplay::updateRenderVolume()
{
    m_pGLWidget->update();
}

CGLWidget*C3dDisplay::getGLWidget()
{
    return m_pGLWidget;
}

void C3dDisplay::onUseRaycasting()
{
    emit doUpdateRenderMode(static_cast<size_t>(RenderMode::RAYCAST));
}

void C3dDisplay::onUseMIP()
{
    emit doUpdateRenderMode(static_cast<size_t>(RenderMode::MIP));
}

void C3dDisplay::onUseMinIP()
{
    emit doUpdateRenderMode(static_cast<size_t>(RenderMode::MinIP));
}

void C3dDisplay::onUseIso()
{
    emit doUpdateRenderMode(static_cast<size_t>(RenderMode::ISOSURFACE));
}

void C3dDisplay::onUpdateOffset(int value)
{
    float val = value/1000.0f;
    emit doUpdateParam(static_cast<size_t>(RenderParamType::OFFSET), val);
}

void C3dDisplay::onExportScreenshot()
{
    auto fileName = Utils::File::saveFile(this, tr("Export screenshot"), "", tr("Image formats(*.jpg, *.png, *.tif)"), QStringList({"jpg", "png", "tif"}), ".png");

    if(fileName.isEmpty())
        return;

    m_pGLWidget->exportScreenshot(fileName);
}

void C3dDisplay::onExportAnimation()
{
    if(m_pAnimationDlg == nullptr)
    {
        m_pAnimationDlg = new C3dAnimationDlg(this);

        connect(m_pAnimationDlg, &C3dAnimationDlg::doStartRecording, [&]{ m_pGLWidget->startAnimationRecording(); });
        connect(m_pAnimationDlg, &C3dAnimationDlg::doStopRecording, [&]{ m_pGLWidget->stopAnimationRecording(); });
        connect(m_pAnimationDlg, &C3dAnimationDlg::doPlayAnimation, [&](const C3dAnimation& animation)
        {
            m_pGLWidget->playAnimation(animation);
        });
        connect(m_pAnimationDlg, &C3dAnimationDlg::doMakeVideoAnimation, [&](const QString& path, const C3dAnimation& animation)
        {
            m_pGLWidget->makeVideoAnimation(path, animation);
        });
        connect(m_pAnimationDlg, &C3dAnimationDlg::accepted, [&]{ delete m_pAnimationDlg; m_pAnimationDlg = nullptr; });
        connect(m_pAnimationDlg, &C3dAnimationDlg::rejected, [&]{ delete m_pAnimationDlg; m_pAnimationDlg = nullptr; });
    }
    m_pAnimationDlg->show();
}
