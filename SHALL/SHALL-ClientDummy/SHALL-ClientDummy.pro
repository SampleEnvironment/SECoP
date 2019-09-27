# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (c) 2017-2019 Helmholtz-Zentrum Berlin fuer Materialien und Energie GmbH <https://www.helmholtz-berlin.de>

lessThan(QT_MAJOR_VERSION, 5) {
    error("need Qt 5.x or higher with C++11 support")
}

CONFIG   +=c++11
QT       += network core gui widgets

TEMPLATE = app
INCLUDEPATH  += ../SHALL-ClientLib
INCLUDEPATH  += ../SHALL-VariantLib

equals(QMAKE_TARGET.arch,x86_64)|equals(QMAKE_TARGET.arch,amd64)|equals(QMAKE_TARGET.arch,x64)|equals(QMAKE_TARGET.arch,ppc64)|equals(QMAKE_TARGET.arch,arm64) {
    CLIENTLIB   = SHALLclient64
    VARIANTLIB = SHALLvariant64
} else {
 equals(QMAKE_TARGET.arch,x86)|equals(QMAKE_TARGET.arch,ppc)|equals(QMAKE_TARGET.arch,arm) {
    CLIENTLIB   = SHALLclient32
    VARIANTLIB = SHALLvariant32
 } else {
  equals(QMAKE_HOST.arch,x86_64)|equals(QMAKE_HOST.arch,amd64)|equals(QMAKE_HOST.arch,x64)|equals(QMAKE_HOST.arch,ppc64)|equals(QMAKE_HOST.arch,arm64) {
    CLIENTLIB   = SHALLclient64
    VARIANTLIB = SHALLvariant64
  } else {
   equals(QMAKE_HOST.arch,x86)|equals(QMAKE_HOST.arch,ppc)|equals(QMAKE_HOST.arch,arm) {
    CLIENTLIB   = SHALLclient32
    VARIANTLIB = SHALLvariant32
   } else {
    CLIENTLIB   = SHALLclient
    VARIANTLIB = SHALLvariant
   }
  }
 }
}

DESTDIR = ../bin
win32-msvc* {
  LIBS += $$quote(../lib/$${CLIENTLIB}.lib) $$quote(../lib/$${VARIANTLIB}.lib)
} else {
  LIBS += $$quote(-L../lib) -l$${CLIENTLIB} -l$${VARIANTLIB}
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES = \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    main.h \
    mainwindow.h \
    ../SHALL-ClientLib/exports.h \
    ../SHALL-VariantLib/SECoP-Variant.h

FORMS += \
    mainwindow.ui
