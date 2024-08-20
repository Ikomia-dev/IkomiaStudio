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
 * @file      CProjectPane.cpp
 * @author    Guillaume Demarcq and Ludovic Barusseau
 * @brief     Implementation file for CProjectPane
 *
 * @details   Details
 */

#include "CProjectPane.h"
#include <QToolBox>
#include <QFileSystemModel>
#include <QFileDialog>
#include <QMessageBox>
#include "CDataIO.hpp"
#include "CDatasetLoadPolicyDlg.h"
#include "Main/LogCategory.h"
#include "Model/Project/CProjectUtils.hpp"
#include "Main/AppTools.hpp"

CProjectPane::CProjectPane(QWidget* parent) : QWidget(parent)
{
    initLayout();
    initContextMenu();
    initConnections();
}

void CProjectPane::setModel(QAbstractItemModel* pModel)
{
    assert(pModel != nullptr);
    m_pTreeView->setModel(pModel);

    // Connect selection model when item changes (keyboard, update image sequence...)
    connect(m_pTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CProjectPane::onSelectionChanged);
}

CProjectTreeView *CProjectPane::getTreeView()
{
    return m_pTreeView;
}

void CProjectPane::initContextMenu()
{
    //Contextual menu for project item
    m_contextMenu.addAction(    TreeItemType::PROJECT,
                                tr("Add folder"),
                                [this](QModelIndex& index){emit doAddEmptyFolder(index); emit doUpdateIndex(index);},
                                QIcon(":/Images/add-folder.png") );
    m_contextMenu.addAction(    TreeItemType::PROJECT,
                                tr("Add Dicom folder"),
                                std::bind(&CProjectPane::selectDicomFolder, this, std::placeholders::_1),
                                QIcon(":/Images/add-folder.png") );
    m_contextMenu.addAction(    TreeItemType::PROJECT,
                                tr("Add dataset"),
                                [this](QModelIndex& index){emit doAddDataset(index, IODataType::IMAGE);emit doUpdateIndex(index);},
                                QIcon(":/Images/add-input.png") );
    m_contextMenu.addAction(    TreeItemType::PROJECT,
                                tr("Add image(s)"),
                                std::bind(&CProjectPane::selectImageFiles, this, std::placeholders::_1),
                                QIcon(":/Images/new-image.png"));
    m_contextMenu.addAction(    TreeItemType::PROJECT,
                                tr("Add folder"),
                                std::bind(&CProjectPane::selectFolder, this, std::placeholders::_1),
                                QIcon(":/Images/folder.png"));
    m_contextMenu.addAction(    TreeItemType::PROJECT,
                                tr("Save project as..."),
                                std::bind(&CProjectPane::saveAs, this, std::placeholders::_1),
                                QIcon(":/Images/save-color-as.png"));
    m_contextMenu.addAction(    TreeItemType::PROJECT,
                                tr("Save project"),
                                std::bind(&CProjectPane::save, this, std::placeholders::_1),
                                QIcon(":/Images/save-color.png"));
    m_contextMenu.addAction(    TreeItemType::PROJECT,
                                tr("Export project"),
                                std::bind(&CProjectPane::exportProject, this, std::placeholders::_1),
                                QIcon(":/Images/export-color.png"));
    m_contextMenu.addAction(    TreeItemType::PROJECT,
                                tr("Close project"),
                                [this](QModelIndex& index){emit doCloseProject(index);},
                                QIcon(":/Images/close-project.png"));

    //Contextual menu for folder item
    m_contextMenu.addAction(    TreeItemType::FOLDER,
                                tr("Add folder"),
                                [this](QModelIndex& index){emit doAddEmptyFolder(index); emit doUpdateIndex(index);},
                                QIcon(":/Images/add-folder.png"));
    m_contextMenu.addAction(    TreeItemType::FOLDER,
                                tr("Add Dicom folder"),
                                std::bind(&CProjectPane::selectDicomFolder, this, std::placeholders::_1),
                                QIcon(":/Images/add-folder.png") );
    m_contextMenu.addAction(    TreeItemType::FOLDER,
                                tr("Add dataset"),
                                [this](QModelIndex& index){emit doAddDataset(index, IODataType::IMAGE); emit doUpdateIndex(index);},
                                QIcon(":/Images/add-input.png") );
    m_contextMenu.addAction(    TreeItemType::FOLDER,
                                tr("Add image(s)"),
                                std::bind(&CProjectPane::selectImageFiles, this, std::placeholders::_1),
                                QIcon(":/Images/new-image.png"));
    m_contextMenu.addAction(    TreeItemType::FOLDER,
                                tr("Remove folder"),
                                [this](QModelIndex& index){ emit doRemoveItem(index); },
                                QIcon(":/Images/delete-color.png"));

    //Contextual menu for dataset item
    m_contextMenu.addAction(    TreeItemType::DATASET,
                                tr("Add image(s)"),
                                std::bind(&CProjectPane::selectImageFiles, this, std::placeholders::_1),
                                QIcon(":/Images/new-image.png"));
    m_contextMenu.addMenu(TreeItemType::DATASET, tr("Add dimension"));
    m_contextMenu.addAction(    TreeItemType::DATASET,
                                tr("Remove dataset"),
                                [this](QModelIndex& index){ emit doRemoveItem(index); },
                                QIcon(":/Images/delete-color.png"));

    //Contextual menu for dimension item
    m_contextMenu.addAction(    TreeItemType::DIMENSION,
                                tr("Add image(s)"),
                                std::bind(&CProjectPane::selectImageFiles, this, std::placeholders::_1),
                                QIcon(":/Images/new-image.png"));
    m_contextMenu.addMenu(TreeItemType::DIMENSION, tr("Add dimension"));
    m_contextMenu.addAction(    TreeItemType::DIMENSION,
                                tr("Remove dimension"),
                                [this](QModelIndex& index){ emit doRemoveItem(index); },
                                QIcon(":/Images/delete-color.png"));

    //Contextual menu for graphics layer item
    m_contextMenu.addAction(    TreeItemType::GRAPHICS_LAYER,
                                tr("Set as active layer"),
                                [this](QModelIndex& index){ emit doSetCurrentGraphicsLayer(index); },
                                QIcon(":/Images/draw-current-layer.png"));
    m_contextMenu.addAction(    TreeItemType::GRAPHICS_LAYER,
                                tr("Add layer"),
                                [this](QModelIndex& index){ emit doAddGraphicsLayer(index); },
                                QIcon(":/Images/draw-new-layer.png"));
    m_contextMenu.addAction(    TreeItemType::GRAPHICS_LAYER,
                                tr("Remove layer"),
                                [this](QModelIndex& index){ emit doRemoveItem(index); },
                                QIcon(":/Images/delete.png"));

    //Contextual menu for image item
    m_contextMenu.addAction(    TreeItemType::IMAGE,
                                tr("Remove image"),
                                [this](QModelIndex& index){ emit doRemoveItem(index); },
                                QIcon(":/Images/delete.png"));
    m_contextMenu.addAction(    TreeItemType::IMAGE,
                                tr("Show location"),
                                [this](QModelIndex& index){ emit doShowLocation(index); },
                                QIcon(":/Images/show-location.png"));


    //Contextual menu for video item
    m_contextMenu.addAction(    TreeItemType::VIDEO,
                                tr("Remove video"),
                                [this](QModelIndex& index){ emit doRemoveItem(index); },
                                QIcon(":/Images/delete.png"));
    m_contextMenu.addAction(    TreeItemType::VIDEO,
                                tr("Show location"),
                                [this](QModelIndex& index){ emit doShowLocation(index); },
                                QIcon(":/Images/show-location.png"));

    //Contextual menu for stream item
    m_contextMenu.addAction(    TreeItemType::LIVE_STREAM,
                                tr("Remove stream"),
                                [this](QModelIndex& index){ emit doRemoveItem(index); },
                                QIcon(":/Images/delete.png"));
}

void CProjectPane::initConnections()
{
    connect(m_pTreeView, &CProjectTreeView::customContextMenuRequested, this, &CProjectPane::onShowContextMenu);
}

void CProjectPane::onShowContextMenu(const QPoint &pos)
{
    QModelIndex index = m_pTreeView->indexAt(pos);
    if(!index.isValid())
        return;

    auto item = static_cast<ProjectTreeItem*>(wrapIndex(index).internalPointer());
    if(item)
    {
        if(item->getTypeId() == TreeItemType::DATASET ||
           item->getTypeId() == TreeItemType::DIMENSION)
        {
            addDimensionSubMenu(index);
        }
        m_contextMenu.displayMenu(item->getTypeId(), m_pTreeView->mapToGlobal(pos), index);
    }
}

void CProjectPane::onApplyProcess(const std::string& processName, const std::shared_ptr<CWorkflowTaskParam> &pParam)
{
    emit doApplyProcess(m_pTreeView->currentIndex(), processName, pParam);
}

void CProjectPane::onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    emit m_pTreeView->dataChanged(topLeft, bottomRight, roles);
}

void CProjectPane::onUpdateSelectedIndex(const QModelIndex& index)
{
    disconnect(m_pTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CProjectPane::onSelectionChanged);
    m_pTreeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    connect(m_pTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CProjectPane::onSelectionChanged);
}

void CProjectPane::onSelectionChanged(const QItemSelection& selected, const QItemSelection&)
{
    // If not empty, selection has changed, we take the first element
    if(selected.indexes().isEmpty() == false)
        onTreeViewClicked(selected.indexes().first());
}

void CProjectPane::selectImageFiles(QModelIndex &index)
{
    if(!index.isValid())
    {
        qWarning(logProject).noquote() << tr("Invalid project item");
        return;
    }

    //Get item corresponding to index
    ProjectTreeItem* pItem = static_cast<ProjectTreeItem*>(wrapIndex(index).internalPointer());
    if(!isValidImageContainerItem(pItem))
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Image(s) can't be added to the dataset"));
        msgBox.setInformativeText(tr("Please check your project structure: \n- Dimension type must be the same at a given tree level.\n- Image(s) must be inserted into the last dimension level."));
        msgBox.exec();
        return;
    }

    QSettings IkomiaSettings;
    QStringList files = QFileDialog::getOpenFileNames(  this, tr("Choose image files"), IkomiaSettings.value(_DefaultDirImg).toString(),
                                                        tr("All images (*.jpg *.JPG *.jpeg *.JPEG *.tif *.TIF *.tiff *.TIFF *.png *.PNG *.bmp *.BMP *.jp2 *.JP2 *.pgm *.PGM *.exr *.EXR *.hdr *.HDR *.ppm *.PPM *.webp *.WEBP)"), nullptr,
                                                        CSettingsManager::dialogOptions() );
    if(files.size() > 0)
    {
        IkomiaSettings.setValue(_DefaultDirImg, QFileInfo(files.first()).path());
        if(pItem->getTypeId() == static_cast<size_t>(TreeItemType::DIMENSION))
            emit doAddImagesToDimension(index, files);
        else
        {
            try
            {
                DatasetLoadPolicy loadPolicy = getDatasetLoadPolicy(pItem->getTypeId(), files);
                emit doAddImages(index, files, loadPolicy);
            }
            catch(std::exception& e)
            {
                qWarning().noquote() << QString::fromStdString(e.what());
            }
        }
    }
    else
        qWarning(logProject).noquote() << tr("No image file selected");
}

void CProjectPane::selectFolder(const QModelIndex &index)
{
    if(!index.isValid())
    {
        qWarning(logProject).noquote() << tr("Invalid project item");
        return;
    }

    QSettings ikomiaSettings;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose image folder"), ikomiaSettings.value(_DefaultDirImg).toString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | CSettingsManager::dialogOptions());
    if(!dir.isEmpty())
    {
        ikomiaSettings.setValue(_DefaultDirImg, dir);
        emit doAddFolder(dir, index);
    }
}

void CProjectPane::selectDicomFolder(QModelIndex &index)
{
    if(!index.isValid())
    {
        qWarning(logProject).noquote() << tr("Invalid project item");
        return;
    }

    QSettings IkomiaSettings;

    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Choose Dicom directory"), IkomiaSettings.value(_DefaultDirDicom).toString(), CSettingsManager::dialogOptions());
    if(folderPath.isEmpty())
        return;

    IkomiaSettings.setValue(_DefaultDirDicom, QFileInfo(folderPath).path());
    emit doAddDicomFolder(index, folderPath);
}

void CProjectPane::initLayout()
{
    // Create project tree view
    m_pTreeView = new CProjectTreeView;
    m_pTreeView->setObjectName("CTreeView");
    m_pTreeView->setHeaderHidden(true);
    m_pTreeView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Create project toolbar
    QToolBar* pToolbar = new QToolBar;
    QAction* pNewProjectAct = new QAction(QIcon(":/Images/add.png"), tr("New project"), this);
    pNewProjectAct->setStatusTip(tr("Create new project"));
    connect(pNewProjectAct, &QAction::triggered, [this]{ emit doNewProject(); });
    pToolbar->addAction(pNewProjectAct);
    pToolbar->widgetForAction(pNewProjectAct)->setObjectName("CToolbarBtn");

    //Add open project to toolbar
    QAction* pOpenProjectAct = new QAction(QIcon(":/Images/open.png"), tr("Open project"), this);
    pOpenProjectAct->setStatusTip(tr("Open project"));
    connect(pOpenProjectAct, &QAction::triggered, [this]{ emit doOpenProject(); });
    pToolbar->addAction(pOpenProjectAct);
    pToolbar->widgetForAction(pOpenProjectAct)->setObjectName("CToolbarBtn");

    //Add save as project to toolbar
    QAction* pSaveAsProject = new QAction(QIcon(":/Images/save-as.png"), tr("Save as project"), this);
    pSaveAsProject->setStatusTip(tr("Save as project"));
    connect(pSaveAsProject, &QAction::triggered, [this]{ saveAs(m_pTreeView->currentIndex()); });
    pToolbar->addAction(pSaveAsProject);
    pToolbar->widgetForAction(pSaveAsProject)->setObjectName("CToolbarBtn");

    //Add save project to toolbar
    QAction* pSaveProject = new QAction(QIcon(":/Images/save.png"), tr("Save project"), this);
    pSaveProject->setStatusTip(tr("Save project"));
    connect(pSaveProject, &QAction::triggered, [this]{ save(m_pTreeView->currentIndex()); });
    pToolbar->addAction(pSaveProject);
    pToolbar->widgetForAction(pSaveProject)->setObjectName("CToolbarBtn");

    //Add close project to toolbar
    QAction* pCloseProject = new QAction(QIcon(":/Images/close-workflow.png"), tr("Close project"), this);
    pCloseProject->setStatusTip(tr("Close project"));
    connect(pCloseProject, &QAction::triggered, [this]{ emit doCloseProject(m_pTreeView->currentIndex()); });
    pToolbar->addAction(pCloseProject);
    pToolbar->widgetForAction(pCloseProject)->setObjectName("CToolbarBtn");

    // Create container widget and layout
    QWidget* pWidget = new QWidget;
    QVBoxLayout* pWidgetLayout = new QVBoxLayout;

    pWidgetLayout->addWidget(pToolbar);
    pWidgetLayout->addWidget(m_pTreeView);
    pWidget->setLayout(pWidgetLayout);

    // Create toolbox and add our container widget
    QToolBox* pToolBox = new QToolBox;
    pToolBox->addItem(pWidget, tr("Projects"));

    QVBoxLayout* pLayout = new QVBoxLayout;
    pLayout->addWidget(pToolBox);
    setLayout(pLayout);
}

void CProjectPane::addDimensionSubMenu(const QModelIndex &index)
{
    auto pItem = static_cast<ProjectTreeItem*>(wrapIndex(index).internalPointer());
    assert(pItem != nullptr);

    std::set<DataDimension>  validDims;
    std::set<DataDimension>  availableDims{
                DataDimension::TIME,
                /*DataDimension::POSITION,
                DataDimension::MODALITY, */
                DataDimension::VOLUME};

    if(pItem->getChildCount() > 0 && pItem->getChild(0)->getTypeId() == TreeItemType::DIMENSION)
    {
        auto pDimItem = pItem->getChild(0)->getNode<std::shared_ptr<CDimensionItem>>();
        if(pDimItem)
            validDims.insert(pDimItem->getDimension());
    }
    else if(pItem->getTypeId() == TreeItemType::DATASET)
        validDims = availableDims;
    else
    {
        validDims = availableDims;
        auto pDimItem = pItem->getNode<std::shared_ptr<CDimensionItem>>();
        auto pDataset = CProjectUtils::getDataset<CMat>(wrapIndex(index));

        //check if current dimension has already child dimension
        if(pDataset && pDataset->hasDimension(pDimItem->getDimension()))
        {
            size_t level = pDataset->getDataInfo().dimensionPosition(pDimItem->getDimension());
            if(level + 1 < pDataset->getDataInfo().dimensionsCount())
            {
                Dimensions dims = pDataset->getDataInfo().dimensions();
                DataDimension dim = dims[level+1].first;

                auto it = availableDims.find(dim);
                if(it != availableDims.end())
                {
                    validDims.clear();
                    validDims.insert(dims[level+1].first);
                }
            }
        }

        if(validDims.size() > 1)
        {
            //Check for already used dimension in ascendants
            auto pTmpItem = pItem;
            QModelIndex tmpIndex = wrapIndex(index);

            while(pTmpItem->getTypeId() != TreeItemType::DATASET)
            {
                auto pDimItem = pTmpItem->getNode<std::shared_ptr<CDimensionItem>>();
                validDims.erase(pDimItem->getDimension());
                tmpIndex = tmpIndex.parent();
                pTmpItem = static_cast<ProjectTreeItem*>(tmpIndex.internalPointer());
            }
        }
    }

    m_contextMenu.clearSubMenu(pItem->getTypeId());
    for(auto it=validDims.begin(); it!=validDims.end(); ++it)
    {
        m_contextMenu.addSubMenuAction(pItem->getTypeId(),
                                       QString::fromStdString(Utils::Data::getDimensionName(*it)),
                                       std::bind(&CProjectPane::addProjectSubDimension, this, std::placeholders::_1, *it));
    }
}

void CProjectPane::addProjectSubDimension(QModelIndex &index, DataDimension dim)
{
    if(!index.isValid())
    {
        qWarning(logProject).noquote() << tr("Invalid project item");
        return;
    }

    //Get item corresponding to index
    ProjectTreeItem* pItem = static_cast<ProjectTreeItem*>(wrapIndex(index).internalPointer());
    if(isItemHasChildData(pItem))
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Warning this folder already contains some data."));
        msgBox.setInformativeText(tr("All data will be move into the new dimension. Are you sure to proceed?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        if(msgBox.exec() == QMessageBox::Yes)
        {
            emit doInsertProjectDimension(index, dim);
            emit doUpdateIndex(index);
        }
    }
    else
    {
        emit doAddDimension(index, dim);
        emit doUpdateIndex(index);
    }
}

DatasetLoadPolicy CProjectPane::getDatasetLoadPolicy(size_t srcType, const QStringList &files)
{
    if(files.size() == 1)
    {
        //1 file = 1 dataset whatever dimensions contained by the file
        return std::make_pair(Relationship::ONE_TO_ONE, DataDimension::NONE);
    }
    else
    {
        //Check if all files contain the same data dimensions
        if(!isSameImageDimensions(files))
            return std::make_pair(Relationship::ONE_TO_ONE, DataDimension::NONE);
        else
        {
            //If yes, ask user to choose the load policy
            // 1 - as many dataset as files
            // 2 - 1 dataset, simple image serie
            // 3 - 1 dataset, extra dimension to be specified
            CDatasetLoadPolicyDlg loadPolicyDlg(static_cast<TreeItemType>(srcType), this);
            if(loadPolicyDlg.exec() == QDialog::Accepted)
                return loadPolicyDlg.getLoadPolicy();
            else
                throw CException(AppExCode::INVALID, tr("Load operation cancelled").toStdString(), __func__, __FILE__, __LINE__);
        }
    }
}

QModelIndex CProjectPane::getCurrentIndex()
{
    return m_pTreeView->currentIndex();
}

bool CProjectPane::isValidImageContainerItem(ProjectTreeItem *pItem)
{
    assert(pItem != nullptr);

    if(pItem->getTypeId() == TreeItemType::DATASET && hasDimensionItem(pItem))
        return false;
    else if(pItem->getTypeId() == TreeItemType::DIMENSION)
        return isLeafDimension(pItem);
    else
        return true;
}

bool CProjectPane::isSameImageDimensions(const QStringList &files)
{
    //Check if all files contain the same data dimensions
    Dimensions refDims = _CDataIO::dimensions(files[0].toStdString());;
    for(int i=1; i<files.size(); ++i)
    {
        Dimensions dims = _CDataIO::dimensions(files[i].toStdString());
        if(dims != refDims)
            return false;
    }
    return true;
}

bool CProjectPane::isLeafDimension(ProjectTreeItem *pItemDim)
{
    assert(pItemDim != nullptr);
    assert(pItemDim->getTypeId() == TreeItemType::DIMENSION);

    for(int i=0; i<pItemDim->getChildCount(); ++i)
    {
        auto pChild = pItemDim->getChild(i);
        if(pChild->getTypeId() == TreeItemType::DIMENSION)
            return false;
    }
    return true;
}

bool CProjectPane::isItemHasChildData(ProjectTreeItem *pItemDim)
{
    assert(pItemDim != nullptr);

    for(int i=0; i<pItemDim->getChildCount(); ++i)
    {
        auto pChild = pItemDim->getChild(i);
        if(pChild->getTypeId() == TreeItemType::IMAGE)
            return true;
    }
    return false;
}

bool CProjectPane::hasDimensionItem(ProjectTreeItem *pItem)
{
    assert(pItem != nullptr);
    for(int i=0; i<pItem->getChildCount(); ++i)
    {
        auto pChild = pItem->getChild(i);
        if(pChild->getTypeId() == TreeItemType::DIMENSION)
            return true;
    }
    return false;
}

void CProjectPane::saveAs(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    QSettings IkomiaSettings;

    auto fileName = Utils::File::saveFile(this, tr("Save Project"), IkomiaSettings.value(_DefaultDirProjectSaveAs).toString(), tr("db Files (*.db)"), QStringList("db"), ".db");
    if(fileName.isEmpty())
        return;

    IkomiaSettings.setValue(_DefaultDirProjectSaveAs, QFileInfo(fileName).path());
    emit doSaveProjectAs(index, fileName);
}

void CProjectPane::save(const QModelIndex& index)
{
    if(!index.isValid())
        return;

    auto pItem = static_cast<CMultiProjectModel::TreeItem*>(index.internalPointer());
    auto pModel = static_cast<CProjectModel*>(pItem->m_pModel);
    assert(pModel);

    if(pModel->getPath().isEmpty())
        saveAs(index);
    else
        emit doSaveProject(index);
}

void CProjectPane::exportProject(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    QSettings IkomiaSettings;

    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Choose export directory"), IkomiaSettings.value(_DefaultDirProjectExport).toString(), CSettingsManager::dialogOptions());
    if(folderPath.isEmpty() == false)
    {
        IkomiaSettings.setValue(_DefaultDirProjectExport, QFileInfo(folderPath).path());
        emit doExportProject(index, folderPath);
    }
}

QModelIndex CProjectPane::wrapIndex(const QModelIndex& index)
{
    auto pModel = static_cast<const CMultiProjectModel*>(index.model());
    auto pTreeItem = static_cast<CMultiProjectModel::TreeItem*>(index.internalPointer());
    return pModel->wrappedIndex(pTreeItem, pTreeItem->m_pModel, index);
}

void CProjectPane::onTreeViewClicked(const QModelIndex& index)
{
    if(index.isValid())
    {
        if(m_currentIndex == index)
            return;

        m_currentIndex = index;

        // Update current selection
        onUpdateSelectedIndex(index);
        // Manage item change to clean previous resources
        emit doUpdateProjectModelIndex(index);
        // Update current index in view
        emit doUpdateIndex(index);
        // Display data
        emit doDisplayData(index);
    }
}

void CProjectPane::onUpdateProject(CMultiProjectModel *pProject)
{
    setModel(pProject);
}

void CProjectPane::onUpdateIndex(const QModelIndex& index)
{
    if(m_currentIndex == index)
        return;

    // Setcurrent notifies selection model which notifies onTreeViewClicked
    m_pTreeView->setCurrentIndex(index);

    // If index is root (QModedlIndex()), force update CPathNavigator to clear
    if(!index.isValid())
        emit doUpdateIndex(index);
}

#include "moc_CProjectPane.cpp"
