from ikomia import core, dataprocess
from ikomia.utils import pyqtutils, qtconversion
from _PluginName_._PluginName__process import _PluginClassName_Param

_QtBindingBlock_


# --------------------
# - Class which implements widget associated with the algorithm
# - Inherits PyCore.CWorkflowTaskWidget from Ikomia API
# --------------------
class _PluginClassName_Widget(_WidgetBaseClass_):

    def __init__(self, param, parent):
        _WidgetBaseClass_.__init__(self, parent)

        if param is None:
            self.parameters = _PluginClassName_Param()
        else:
            self.parameters = param

        _InitQtLayout_
        # Set widget layout
        self.set_layout(layout_ptr)

    def on_apply(self):
        # Apply button clicked slot

        # Get parameters from widget
        # Example : self.parameters.window_size = self.spin_window_size.value()

        # Send signal to launch the algorithm main function
        self.emit_apply(self.parameters)


# --------------------
# - Factory class to build algorithm widget object
# - Inherits PyDataProcess.CWidgetFactory from Ikomia API
# --------------------
class _PluginClassName_WidgetFactory(dataprocess.CWidgetFactory):

    def __init__(self):
        dataprocess.CWidgetFactory.__init__(self)
        # Set the algorithm name attribute -> it must be the same as the one declared in the algorithm factory class
        self.name = "_PluginName_"

    def create(self, param):
        # Create widget object
        return _PluginClassName_Widget(param, None)
