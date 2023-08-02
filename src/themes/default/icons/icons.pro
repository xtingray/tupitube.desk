INSTALLS = icons 

icons.target = .
icons.commands = chmod 755 *png; cp *.png $(INSTALL_ROOT)/themes/default/icons
icons.path = /themes/default/icons

CONFIG += warn_on

TEMPLATE = subdirs
