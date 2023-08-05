INSTALLS = hd

hd.target = .
hd.commands = chmod 755 *png; cp *.png $(INSTALL_ROOT)/themes/default/icons/hd
hd.path = /themes/default/icons/hd

CONFIG += warn_on

TEMPLATE = subdirs
