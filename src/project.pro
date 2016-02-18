TEMPLATE = subdirs
SUBDIRS = core \
          gui

# build must be last:
CONFIG += ordered
SUBDIRS += build
