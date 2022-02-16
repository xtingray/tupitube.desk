INSTALLS = images 

images.target = .
images.commands = cp *.png $(INSTALL_ROOT)/themes/light/images
images.path = /themes/light/images

CONFIG += warn_on

TEMPLATE = subdirs
