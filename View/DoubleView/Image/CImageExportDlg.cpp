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

#include "CImageExportDlg.h"
#include <QMessageBox>
#include "Main/AppTools.hpp"

CImageExportDlg::CImageExportDlg(const QString& title, DataType type, QWidget *parent, Qt::WindowFlags f)
    : CDialog(title, parent, DEFAULT|EFFECT_ENABLED, f)
{
    m_dataType = type;
    initLayout();
    initConnections();
}

QString CImageExportDlg::getFileName() const
{
    return m_pEditPath->text();
}

bool CImageExportDlg::isGraphicsExported() const
{
    return m_pCheckBurnGraphics->isChecked();
}

void CImageExportDlg::initLayout()
{
    m_pCheckBurnGraphics = new QCheckBox(tr("Embed graphics layers"));
    auto pLabelPath = new QLabel(tr("Path"));
    m_pEditPath = new QLineEdit;
    m_pBrowseBtn = new QPushButton("...");
    m_pBrowseBtn->setToolTip(tr("Select target folder"));

    auto pGridLayout = new QGridLayout;
    pGridLayout->addWidget(m_pCheckBurnGraphics, 0, 0, 1, 3);
    pGridLayout->addWidget(pLabelPath, 1, 0);
    pGridLayout->addWidget(m_pEditPath, 1, 1);
    pGridLayout->addWidget(m_pBrowseBtn, 1, 2);

    m_pOkBtn = new QPushButton(tr("OK"));
    m_pOkBtn->setDefault(true);
    m_pCancelBtn = new QPushButton(tr("Cancel"));

    QHBoxLayout* pBtnLayout = new QHBoxLayout;
    pBtnLayout->addWidget(m_pOkBtn);
    pBtnLayout->addWidget(m_pCancelBtn);

    auto pLayout = getContentLayout();
    pLayout->addSpacing(5);
    pLayout->addLayout(pGridLayout);
    pLayout->addLayout(pBtnLayout);
}

void CImageExportDlg::initConnections()
{
    connect(m_pBrowseBtn, &QPushButton::clicked, this, &CImageExportDlg::onBrowse);
    connect(m_pOkBtn, &QPushButton::clicked, this, &CImageExportDlg::onValidate);
    connect(m_pCancelBtn, &QPushButton::clicked, this, &CImageExportDlg::reject);
}

void CImageExportDlg::onBrowse()
{
    QString path;
    QSettings IkomiaSettings;

    switch(m_dataType)
    {
        case DataType::IMAGE:
            path = Utils::File::saveFile(this, tr("Export screenshot"),
                                             IkomiaSettings.value(_DefaultDirImgExport).toString(),
                                             tr("All images (*.jpg *.jpeg *.tif *.tiff *.png *.bmp *.jp2)"),
                                             QStringList({"jpg", "jpeg", "tif", "tiff", "png", "bmp", "jp2"}), ".png");
            if(path.isEmpty())
                return;

            IkomiaSettings.setValue(_DefaultDirImgExport, QFileInfo(path).path());
            break;

        case DataType::IMAGE_SEQUENCE:
            path = QFileDialog::getExistingDirectory(this, tr("Save image sequence"), IkomiaSettings.value(_DefaultDirVideoExport).toString());
            if(path.isEmpty())
                return;

            IkomiaSettings.setValue(_DefaultDirVideoExport, QFileInfo(path).path());
            break;

        case DataType::VIDEO:
            path = Utils::File::saveFile(this, tr("Save Video"),
                                         IkomiaSettings.value(_DefaultDirVideoExport).toString(),
                                         tr("avi Files (*.avi)"),
                                         QStringList("avi"), ".avi");
            if(path.isEmpty())
                return;

            IkomiaSettings.setValue(_DefaultDirVideoExport, QFileInfo(path).path());
            break;
    }
    m_pEditPath->setText(path);
}

void CImageExportDlg::onValidate()
{
    if(m_pEditPath->text().isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Please enter a valid path"));
        msgBox.exec();
        return;
    }
    QDialog::accept();
}
