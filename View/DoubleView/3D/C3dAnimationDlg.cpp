#include "C3dAnimationDlg.h"
#include <QGridLayout>
#include <QComboBox>
#include <QListView>
#include <QStringListModel>
#include <QFileDialog>
#include "Main/AppTools.hpp"

C3dAnimationDlg::C3dAnimationDlg(QWidget *parent, Qt::WindowFlags f) :
    CDialog(tr("Build 3D animation"), parent, DEFAULT|EFFECT_ENABLED, f)
{
    initLayout();
    initConnections();
}

void C3dAnimationDlg::initLayout()
{
    QLabel* pLabelFps = new QLabel(tr("Frame per second"));
    m_pEditFps = new QLineEdit("30");

    auto pHLayout1 = new QHBoxLayout;
    pHLayout1->addWidget(pLabelFps);
    pHLayout1->addWidget(m_pEditFps);
    pHLayout1->addStretch(1);

    QLabel* pLabelAction = new QLabel(tr("Action"));

    m_pComboAction = new QComboBox;
    m_pComboAction->addItem(tr("Rotate up"), C3dAnimationSequence::ROTATE_UP);
    m_pComboAction->addItem(tr("Rotate down"), C3dAnimationSequence::ROTATE_DOWN);
    m_pComboAction->addItem(tr("Rotate left"), C3dAnimationSequence::ROTATE_LEFT);
    m_pComboAction->addItem(tr("Rotate right"), C3dAnimationSequence::ROTATE_RIGHT);
    m_pComboAction->addItem(tr("Move along x"), C3dAnimationSequence::TRANSLATE_X);
    m_pComboAction->addItem(tr("Move along y"), C3dAnimationSequence::TRANSLATE_Y);
    m_pComboAction->addItem(tr("Zoom in/out"), C3dAnimationSequence::ZOOM);
    m_pComboAction->addItem(tr("Free move"), C3dAnimationSequence::FREE_MOVE);
    m_pComboAction->setCurrentIndex(0);

    m_pLabelParam = new QLabel(tr("Angle(°)"));
    m_pEditParam = new QLineEdit("90");
    m_pBtnAdd = createButton(tr("Add sequence"), QIcon(":/Images/add.png"));

    auto pHLayout2 = new QHBoxLayout;
    pHLayout2->addWidget(pLabelAction);
    pHLayout2->addWidget(m_pComboAction);
    pHLayout2->addWidget(m_pLabelParam);
    pHLayout2->addWidget(m_pEditParam);
    pHLayout2->addWidget(m_pBtnAdd);
    pHLayout2->addStretch(1);

    m_pAnimationView = new QListView;
    m_pAnimationView->setSelectionMode(QListView::SingleSelection);

    m_pBtnPlay = createButton(tr("Play"), QIcon(":/Images/play.png"));
    m_pBtnRemove = createButton(tr("Remove"), QIcon(":/Images/delete.png"));
    m_pBtnClear = createButton(tr("Clear"), QIcon(":/Images/delete.png"));
    m_pBtnExport = createButton(tr("Export"), QIcon(":/Images/export.png"));
    m_pBtnLoad = createButton(tr("Load"), QIcon(":/Images/open.png"));

    auto pGridLayout = new QGridLayout;
    pGridLayout->addWidget(m_pAnimationView, 0, 0, 5, 1);
    pGridLayout->addWidget(m_pBtnPlay, 0, 1, 1, 1);
    pGridLayout->addWidget(m_pBtnRemove, 1, 1, 1, 1);
    pGridLayout->addWidget(m_pBtnClear, 2, 1, 1, 1);
    pGridLayout->addWidget(m_pBtnExport, 3, 1, 1, 1);
    pGridLayout->addWidget(m_pBtnLoad, 4, 1, 1, 1);

    m_pBtnCreate = new QPushButton(tr("Create"));
    m_pBtnCancel = new QPushButton(tr("Cancel"));

    auto pHLayout3 = new QHBoxLayout;
    pHLayout3->addWidget(m_pBtnCreate);
    pHLayout3->addWidget(m_pBtnCancel);

    auto pVLayout = getContentLayout();
    pVLayout->addLayout(pHLayout1);
    pVLayout->addLayout(pHLayout2);
    pVLayout->addLayout(pGridLayout);
    pVLayout->addLayout(pHLayout3);
}

void C3dAnimationDlg::initConnections()
{
    connect(m_pEditFps, &QLineEdit::editingFinished, [&]{ m_animation.setFps(m_pEditFps->text().toInt()); });
    connect(m_pComboAction, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &C3dAnimationDlg::onActionChanged);
    connect(m_pBtnAdd, &QPushButton::clicked, this, &C3dAnimationDlg::onAddSequence);
    connect(m_pBtnPlay, &QPushButton::clicked, this, &C3dAnimationDlg::onPlayAnimation);
    connect(m_pBtnRemove, &QPushButton::clicked, this, &C3dAnimationDlg::onRemoveSequence);
    connect(m_pBtnClear, &QPushButton::clicked, this, &C3dAnimationDlg::onClearAnimation);
    connect(m_pBtnExport, &QPushButton::clicked, this, &C3dAnimationDlg::onExportAnimation);
    connect(m_pBtnLoad, &QPushButton::clicked, this, &C3dAnimationDlg::onLoadAnimation);
    connect(m_pBtnCreate, &QPushButton::clicked, this, &C3dAnimationDlg::onCreateAnimation);
    connect(m_pBtnCancel, &QPushButton::clicked, this, &C3dAnimationDlg::reject);
}

QPushButton* C3dAnimationDlg::createButton(const QString& text, const QIcon& icon)
{
    auto pBtn = new QPushButton;
    pBtn->setIcon(icon);
    pBtn->setToolTip(text);
    return pBtn;
}

QString C3dAnimationDlg::getSequenceText(const C3dAnimationSequence& sequence) const
{
    QString text;

    switch(sequence.getType())
    {
        case C3dAnimationSequence::ROTATE_UP:
            text = QString(tr("Rotation of %1° from bottom to top").arg(sequence.m_moves.size()));
            break;
        case C3dAnimationSequence::ROTATE_DOWN:
            text = QString(tr("Rotation of %1°from top to bottom").arg(sequence.m_moves.size()));
            break;
        case C3dAnimationSequence::ROTATE_LEFT:
            text = QString(tr("Rotation of %1° from right to left").arg(sequence.m_moves.size()));
            break;
        case C3dAnimationSequence::ROTATE_RIGHT:
            text = QString(tr("Rotation of %1° from left to right").arg(sequence.m_moves.size()));
            break;
        case C3dAnimationSequence::TRANSLATE_X:
            text = QString(tr("Move scene along x-axis by %1 pixels").arg(sequence.m_moves.size()));
            break;
        case C3dAnimationSequence::TRANSLATE_Y:
            text = QString(tr("Move scene along x-axis by %1 pixels").arg(sequence.m_moves.size()));
            break;
        case C3dAnimationSequence::ZOOM:
            text = QString(tr("Move scene along z-axis by %1 pixels").arg(sequence.m_moves.size()));
            break;
        case C3dAnimationSequence::FREE_MOVE:
            text = tr("Free move");
            break;
    }
    return text;
}

void C3dAnimationDlg::addAnimationSequence(const C3dAnimationSequence &sequence)
{
    m_animation.addSequence(sequence);
}

void C3dAnimationDlg::onActionChanged(int index)
{
    auto action = static_cast<C3dAnimationSequence::Type>(m_pComboAction->itemData(index).toInt());
    switch(action)
    {
        case C3dAnimationSequence::ROTATE_UP:
        case C3dAnimationSequence::ROTATE_DOWN:
        case C3dAnimationSequence::ROTATE_LEFT:
        case C3dAnimationSequence::ROTATE_RIGHT:
            m_pLabelParam->setVisible(true);
            m_pEditParam->setVisible(true);
            m_pBtnAdd->setIcon(QIcon(":/Images/add.png"));
            m_pBtnAdd->setToolTip(tr("Add sequence"));
            m_pLabelParam->setText(tr("Angle(°)"));
            m_pEditParam->setText("90");
            break;
        case C3dAnimationSequence::TRANSLATE_X:
        case C3dAnimationSequence::TRANSLATE_Y:
        case C3dAnimationSequence::ZOOM:
            m_pLabelParam->setVisible(true);
            m_pEditParam->setVisible(true);
            m_pBtnAdd->setIcon(QIcon(":/Images/add.png"));
            m_pBtnAdd->setToolTip(tr("Add sequence"));
            m_pLabelParam->setText(tr("Distance (px)"));
            m_pEditParam->setText("50");
            break;
        case C3dAnimationSequence::FREE_MOVE:
            m_pLabelParam->setVisible(false);
            m_pEditParam->setVisible(false);
            m_pBtnAdd->setIcon(QIcon(":/Images/record.png"));
            m_pBtnAdd->setToolTip(tr("Record"));
            break;
    }
}

void C3dAnimationDlg::onAddSequence()
{
    auto action = static_cast<C3dAnimationSequence::Type>(m_pComboAction->currentData().toInt());
    if(action == C3dAnimationSequence::FREE_MOVE)
    {
        if(m_bRecording == false)
        {
            m_bRecording = true;
            m_pBtnAdd->setIcon(QIcon(":/Images/stop-record.png"));
            m_pBtnAdd->setToolTip(tr("Stop record"));
            emit doStartRecording();
            return;
        }
        else
        {
            m_bRecording = false;
            m_pBtnAdd->setIcon(QIcon(":/Images/record.png"));
            m_pBtnAdd->setToolTip(tr("Record"));
            emit doStopRecording();
        }
    }
    else
        m_animation.addSequence(action, m_pEditParam->text().toInt());

    if(m_pAnimationModel == nullptr)
    {
        m_pAnimationModel = new QStringListModel;
        m_pAnimationView->setModel(m_pAnimationModel);
    }

    QString sequenceText = getSequenceText(m_animation.m_sequences.back());
    auto stringList = m_pAnimationModel->stringList();
    stringList.push_back(sequenceText);
    m_pAnimationModel->setStringList(stringList);
}

void C3dAnimationDlg::onPlayAnimation()
{
    emit doPlayAnimation(m_animation);
}

void C3dAnimationDlg::onRemoveSequence()
{
    if(m_pAnimationModel)
    {
        auto itemIndex= m_pAnimationView->currentIndex();
        if(itemIndex.isValid() && itemIndex.row() < m_pAnimationModel->stringList().size())
        {
            m_animation.removeSequence(itemIndex.row());
            m_pAnimationModel->removeRow(itemIndex.row());
        }
    }
}

void C3dAnimationDlg::onClearAnimation()
{
    if(m_pAnimationModel)
    {
        delete m_pAnimationModel;
        m_pAnimationModel = nullptr;
    }
    m_pAnimationView->setModel(nullptr);
    m_animation.clear();
}

void C3dAnimationDlg::onExportAnimation()
{
    auto fileName = Utils::File::saveFile(this, tr("Export animation"), "", tr("Animation file(*.json)"), QStringList("json"), ".json");

    if(fileName.isEmpty())
        return;

    m_animation.save(fileName);
}

void C3dAnimationDlg::onLoadAnimation()
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Load animation"), "", tr("Animation file(*.json)"), nullptr, CSettingsManager::dialogOptions());
    if(fileName.isEmpty() == false)
    {
        m_animation.clear();
        delete m_pAnimationModel;
        m_animation.load(fileName);
        m_pEditFps->setText(QString::number(m_animation.getFps()));
        m_pAnimationModel = new QStringListModel;

        QStringList stringList;
        for(int i=0; i<m_animation.m_sequences.size(); ++i)
            stringList.append(getSequenceText(m_animation.m_sequences[i]));

        m_pAnimationModel->setStringList(stringList);
        m_pAnimationView->setModel(m_pAnimationModel);
    }
}

void C3dAnimationDlg::onCreateAnimation()
{
    auto fileName = Utils::File::saveFile(this, tr("Choose destination file"), "", tr("Video(*.avi)"), QStringList("avi"), ".avi");

    if(fileName.isEmpty())
        return;

    emit doMakeVideoAnimation(fileName, m_animation);
    accept();
}
