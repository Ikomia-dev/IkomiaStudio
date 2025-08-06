"""
Module that implements the UI widget of the algorithm.
"""
from ikomia import core, dataprocess
from ikomia.utils import pyqtutils, qtconversion
from _PluginName_._PluginName__process import _PluginClassName_Param

_QtBindingBlock_


class _PluginClassName_Widget(_WidgetBaseClass_):
    """
    Class that implements UI widget to adjust algorithm parameters.
    Inherits PyCore.CWorkflowTaskWidget from Ikomia API.
    """
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
        """QT slot called when users click the Apply button."""
        # Get parameters from widget
        # Example : self.parameters.window_size = self.spin_window_size.value()

        # Send signal to launch the algorithm main function
        self.emit_apply(self.parameters)


class _PluginClassName_WidgetFactory(dataprocess.CWidgetFactory):
    """
    Factory class to create algorithm widget object.
    Inherits PyDataProcess.CWidgetFactory from Ikomia API.
    """
    def __init__(self):
        dataprocess.CWidgetFactory.__init__(self)
        # Set the algorithm name attribute -> it must be the same as the one declared in the algorithm factory class
        self.name = "_PluginName_"

    def create(self, param):
        """Instantiate widget object."""
        return _PluginClassName_Widget(param, None)
