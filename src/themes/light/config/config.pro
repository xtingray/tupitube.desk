INSTALLS = config 

config.target = .
config.commands = cp *.qss $(INSTALL_ROOT)/themes/light/config
config.path = /themes/light/config

CONFIG += warn_on

TEMPLATE = subdirs
