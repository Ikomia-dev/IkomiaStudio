from ikomia import utils, core, dataprocess
import _PluginName__process as processMod

_QtBindingBlock_


# --------------------
# - Class which implements widget associated with the process
# - Inherits PyCore.CProtocolTaskWidget from Ikomia API
# --------------------
class _PluginName_Widget(_WidgetBaseClass_):

    def __init__(self, param, parent):
        _WidgetBaseClass_.__init__(self, parent)

        if param is None:
            self.parameters = processMod._PluginName_Param()
        else:
            self.parameters = param

        _InitQtLayout_
        # Set widget layout
        self.setLayout(layout_ptr)

    def onApply(self):
        # Apply button clicked slot

        # Get parameters from widget
        # Example : self.parameters.windowSize = self.spinWindowSize.value()

        # Send signal to launch the process
        self.emitApply(self.parameters)


# --------------------
# - Factory class to build process widget object
# - Inherits PyDataProcess.CWidgetFactory from Ikomia API
# --------------------
class _PluginName_WidgetFactory(dataprocess.CWidgetFactory):

    def __init__(self):
        dataprocess.CWidgetFactory.__init__(self)
        # Set the name of the process -> it must be the same as the one declared in the process factory class
        self.name = "_PluginName_"

    def create(self, param):
        # Create widget object
        return _PluginName_Widget(param, None)
