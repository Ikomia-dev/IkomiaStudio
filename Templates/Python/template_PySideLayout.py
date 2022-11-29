# Create layout : QGridLayout by default
        self.grid_layout = QtWidgets.QGridLayout()
        # PySide -> Qt wrapping
        layout_ptr = qtconversion.PySideToQt(self.grid_layout)
