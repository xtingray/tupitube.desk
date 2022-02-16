INSTALLS = icons 

icons.target = .
icons.commands = cp *.png $(INSTALL_ROOT)/themes/light/icons
icons.path = /themes/light/icons

CONFIG += warn_on

TEMPLATE = subdirs
