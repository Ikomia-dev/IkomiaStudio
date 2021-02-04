#include "_PluginName_.h"

//------------------------------
//----- C_PluginName_Param
//------------------------------
C_PluginName_Param::C_PluginName_Param() : CProtocolTaskParam()
{
}

void C_PluginName_Param::setParamMap(const UMapString& paramMap)
{
    Q_UNUSED(paramMap);
    //Set parameters values from Ikomia application
    //Each parameter is mapped to a string for identification
    //Example : m_firstParameter = std::stoi(paramMap.at("firstParameter"));
}

UMapString C_PluginName_Param::getParamMap() const
{
    //Send parameters values to Ikomia application
    UMapString map;
    //Example : map.insert(std::make_pair("firstParameter", std::to_string(m_firstParameter)));
    return map;
}


//------------------------------
//----- C_PluginName_
//------------------------------
C_PluginName_::C_PluginName_() : _ProcessBaseClass_()
{
    //Add input/output of the process here
    //Example :
    //addInput(std::make_shared<CImageProcessIO>());
    //addOutput(std::make_shared<CImageProcessIO>());
}

C_PluginName_::C_PluginName_(const std::string& name, const std::shared_ptr<C_PluginName_Param>& pParam) : _ProcessBaseClass_(name)
{
    m_pParam = std::make_shared<C_PluginName_Param>(*pParam);

    //Add input/output of the process here
    //Example :
    //addInput(std::make_shared<CImageProcessIO>());
    //addOutput(std::make_shared<CImageProcessIO>());
}

size_t C_PluginName_::getProgressSteps()
{
    return 1;
}

void C_PluginName_::run()
{
    //Core function of your process

    //Call beginTaskRun for initialization
    beginTaskRun();
        
    //Examples :
    //Get input : auto pInput = std::dynamic_pointer_cast<CImageProcessIO>(getInput(0));
    //Get output : auto pOutput = std::dynamic_pointer_cast<CImageProcessIO>(getOutput(0));
    //Get parameters : auto pParam = std::dynamic_pointer_cast<C_PluginName_Param>(m_pParam);
    //Get image from input/output: CMat srcImage = pInput->getImage() -> srcImage is a cv::Mat based object
    //Set image of input/output: pOutput->setImage(srcImage)

    //Step progress bar
    emit m_signalHandler->doProgress();

    //Call endTaskRun to finalize process       
    endTaskRun();
}

//------------------------------
//----- C_PluginName_Widget
//------------------------------
C_PluginName_Widget::C_PluginName_Widget(QWidget *parent) : _WidgetBaseClass_(parent)
{
    init();
}

C_PluginName_Widget::C_PluginName_Widget(ProtocolTaskParamPtr pParam, QWidget *parent) : _WidgetBaseClass_(parent)
{
    //Create specific parameters object
    m_pParam = std::dynamic_pointer_cast<C_PluginName_Param>(pParam);
    //Initialize the widget
    init();
}

void C_PluginName_Widget::init()
{
    if(m_pParam == nullptr)
        m_pParam = std::make_shared<C_PluginName_Param>();

    //Build your custom layout here

    connect(m_pApplyBtn, &QPushButton::pressed, [&]{ emit doApplyProcess(m_pParam); } );
}
