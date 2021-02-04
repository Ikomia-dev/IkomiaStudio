#include "CWizardManager.h"
#include "Main/LogCategory.h"
#include "../Project/CProjectManager.h"
#include "View/Common/CBubbleTip.h"
#include "Tutorials/CScenarioSimpleImage.hpp"
#include "Tutorials/CTutoOpticalFlow.hpp"
#include "Tutorials/CTutoStartingHelper.hpp"
#include "CWizardStepModel.h"

CWizardManager::CWizardManager()
{
}

void CWizardManager::setModelView(CMainModel* pModel, CMainView* pView)
{
    assert(pModel && pView);
    m_pModel = pModel;
    m_pView = pView;
}

void CWizardManager::initFactory()
{
    registerFactory(std::make_shared<CTutoStartingHelperFactory>());
    registerFactory(std::make_shared<CScenarioSimpleImageFactory>());
    registerFactory(std::make_shared<CTutoOpticalFlowFactory>());
}

void CWizardManager::registerFactory(const TutorialFactoryPtr& factory)
{
    m_tutorialFactory.getList().push_back(factory);
    auto pTutoFunc = [this, factory]{ return factory->create(m_pModel, m_pView); };
    m_tutorialFactory.registerCreator(factory->getInfo().m_name, pTutoFunc);
}

CBubbleTip* CWizardManager::createBubble(int flags)
{
    CBubbleTip* pBubbleTip = new CBubbleTip(nullptr, m_pView, flags);
    pBubbleTip->setCheckText(tr("Don't show again."));

    if(flags & CBubbleTip::Check)
    {
        connect(pBubbleTip, &CBubbleTip::doChecked, [this](bool isChecked)
        {
            setTutoEnabled(!isChecked);
            m_pModel->getSettingsManager()->enableTutorial(!isChecked);
        });
    }

    if(flags & CBubbleTip::Close)
        connect(pBubbleTip, &CBubbleTip::finished, this, &CWizardManager::reset);

    if(flags & CBubbleTip::Next)
        connect(pBubbleTip, &CBubbleTip::doNext, this, &CWizardManager::onNext);

    return pBubbleTip;
}

void CWizardManager::playTuto(const QString& name)
{
    assert(name.isEmpty() == false);

    m_pScenario = m_tutorialFactory.createObject(name.toStdString());

    assert(m_pScenario);

    connect(m_pScenario, &CWizardScenario::doNext, this, &CWizardManager::onNext);

    // Load selected tutorial
    m_nextId = 0;
    m_stepList = m_pScenario->getScenario();

    if(m_pStepModel != nullptr)
        delete m_pStepModel;

    m_pStepModel = new CWizardStepModel(m_stepList);

    emit doSetStepModel(m_pStepModel);
    emit doSetTotalSteps(m_stepList.size());

    makeTutorial();
}

void CWizardManager::setTutoEnabled(bool bEnable)
{
    m_bTuto = bEnable;
}

void CWizardManager::showBubble(QWidget* pWidget, const QString& title, const QString& text, const QString& stylesheet, int flags)
{
    if(m_pBubbleTip)
        m_pBubbleTip->deleteLater();

    m_pBubbleTip = createBubble(flags);
    showBubble(m_pBubbleTip, pWidget, title, text, stylesheet);
}

void CWizardManager::showBubble(CBubbleTip* pBubbleTip, QWidget* pWidget, const QString& title, const QString& text, const QString& stylesheet)
{
    pBubbleTip->setParent(pWidget);
    pBubbleTip->setAppParent(m_pView);
    pBubbleTip->setTitle(title);
    pBubbleTip->setText(text);
    pBubbleTip->setIcon(QStyle::SP_MessageBoxInformation);
    pBubbleTip->update();
    pBubbleTip->show();

    //QScreen* pScreen = pWidget->window()->windowHandle()->screen();
    //QSize desktopSize = pScreen->availableSize();
    QSize desktopSize = m_pView->size();
    QPoint pos = pWidget->mapTo(m_pView, pWidget->rect().center());
    QPoint translate_pos;
    if ( pos.x() < desktopSize.width() / 2 )
    {
        if ( pos.y() < desktopSize.height() / 2 )
            translate_pos = pWidget->rect().bottomRight();
        else
            translate_pos = pWidget->rect().topRight();
    }
    else
    {
        if ( pos.y() < desktopSize.height() / 2 )
            translate_pos = pWidget->rect().bottomLeft();
        else
            translate_pos = pWidget->rect().topLeft();
    }
    pBubbleTip->setCurrentPos(pos);
    pBubbleTip->move(pWidget->mapToGlobal(translate_pos));

    // Highlight widget by changing styleSheet and keeping original styleSheet in memory
    m_originalStyleSheet = pWidget->styleSheet();
    pWidget->setStyleSheet(m_originalStyleSheet+stylesheet);
}

void CWizardManager::initTutorials()
{
    try
    {
        m_bTuto = m_pModel->getSettingsManager()->isTutorialEnabled();
        initFactory();

        m_db.initDb();
        for(auto&& it : m_tutorialFactory.getList())
            m_db.addTutorial(it->getInfo());

        createWizardQueryModel();
    }
    catch(std::exception& e)
    {
        qCCritical(logWizard()).noquote() << QString::fromStdString(e.what());
    }
}

void CWizardManager::createWizardQueryModel()
{
    if(m_pWizardModel != nullptr)
        delete m_pWizardModel;

    m_pWizardModel = new CWizardQueryModel;
    m_pWizardModel->setQuery(m_db.getAllTutorialsQuery(), m_db.getTutorialDatabase());

    emit doSetTutorialModel(m_pWizardModel);
}

void CWizardManager::updateItemIcon()
{
    if(m_nextId <= 0)
        return;

    QStandardItem* pItem = m_pStepModel->item(m_nextId-1);
    pItem->setIcon(QIcon(":/Images/checkGreen.png"));
    pItem->setCheckState(Qt::Unchecked);
}

void CWizardManager::updateItemState()
{
    if(m_nextId <= 0)
        return;

    QStandardItem* pItem = m_pStepModel->item(m_nextId-1);
    pItem->setCheckState(Qt::Checked);
}

void CWizardManager::reset()
{
    // Restore original styleSheet
    m_pCurrentWidget->setStyleSheet(m_originalStyleSheet);
    // Clear connections
    for(int i=0; i<m_nextId; ++i)
    {
        auto step = m_stepList[i];
        if(step.pSlot == nullptr)
            disconnect(step.pSender, step.pSignal, this, SLOT(onNext()));
        else
            disconnect(step.pSender, step.pSignal, m_pScenario, step.pSlot);
    }

    // Clear step list
    m_stepList.clear();
    m_nextId = 0;
    disconnect(m_pScenario, &CWizardScenario::doNext, this, &CWizardManager::onNext);
}

void CWizardManager::launchTutorialHelper()
{
    if(!m_bTuto)
        return;

    QString title = tr("First step in Ikomia?");
    QString text = tr("You don't know how to start with Ikomia? "
                      "Feel free to choose any of our interactive tutorials!");

    QString stylesheet = "border: 2px solid red; border-radius: 5px;";

    auto pWidget = m_pView->getBtn(CMainView::BTN_TUTO_PANE);

    CBubbleTip* pBubbleTip = new CBubbleTip(pWidget, m_pView, CBubbleTip::Close | CBubbleTip::Check);
    pBubbleTip->setAttribute(Qt::WA_DeleteOnClose);
    pBubbleTip->setCheckText(tr("Don't show again."));

    showBubble(pBubbleTip, pWidget, title, text, stylesheet);

    connect(pBubbleTip, &CBubbleTip::doChecked, [this](bool isChecked)
    {
        setTutoEnabled(!isChecked);
        m_pModel->getSettingsManager()->enableTutorial(!isChecked);
    });

    connect(pBubbleTip, &CBubbleTip::finished, [this, pWidget]{ pWidget->setStyleSheet(m_originalStyleSheet); });
}

void CWizardManager::launchFirstStepHelper()
{
    if(!m_bTuto)
        return;

    playTuto("IkomiaStart");
}

void CWizardManager::makeTutorial()
{
    // Check if we have at least one step
    if(m_stepList.isEmpty())
        return;

    assert(m_nextId < m_stepList.size());
    // Run first step
    run(m_stepList[m_nextId++]);
}

void CWizardManager::run(WizardStep step)
{
    emit doUpdateStepPage(step.text, m_nextId, step.description);
    updateItemState();

    // Store the current widget for reuse
    m_pCurrentWidget = step.pWidget;

    // Show information bubble
    showBubble(m_pCurrentWidget, "Step "+QString::number(m_nextId), step.text, step.styleSheet, step.flags);

    // Create a single shot connection to avoid problems during scenarios
    // (i.e. a button click must behave once)
    if(m_pSingleShotConnection)
    {
        QObject::disconnect(*m_pSingleShotConnection);
        delete m_pSingleShotConnection;
    }

    m_pSingleShotConnection = new QMetaObject::Connection();

    // If a custom slot exist, connect to it otherwise connect to onNext()
    if(step.pSlot == nullptr)
    {
        *m_pSingleShotConnection = connect(step.pSender, step.pSignal, this, SLOT(onNext()));;
    }
    else
        *m_pSingleShotConnection = connect(step.pSender, step.pSignal, m_pScenario, step.pSlot);
}

void CWizardManager::onNext()
{
    // Close bubble tip if not already
    m_pBubbleTip->hide();
    // Restore original styleSheet
    m_pCurrentWidget->setStyleSheet(m_originalStyleSheet);
    // Go to next step
    if(m_nextId<m_stepList.size())
    {
        updateItemIcon();
        run(m_stepList[m_nextId++]);
    }
    else
    {
        updateItemIcon();
        updateItemState();
        reset();
        //emit doFinish();
    }
}

void CWizardManager::onCancel()
{
    // Close bubble tip if not already
    m_pBubbleTip->close();

    reset();
    emit doFinish();
}

void CWizardManager::onPlayTuto(const QModelIndex& index)
{
    QString name = m_pWizardModel->record(index.row()).value("name").toString();

    playTuto(name);
}

#include "moc_CWizardManager.cpp"
