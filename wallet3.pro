QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    account_edit.cpp \
    account_widget.cpp \
    add_account_window.cpp \
    add_category_window.cpp \
    add_inc_dec.cpp \
    add_transfer_window.cpp \
    binary_loader.cpp \
    domain.cpp \
    first_window.cpp \
    main.cpp \
    mainwindow.cpp \
    model.cpp \
    model_representation.cpp \
    serialization.cpp \
    transaction_editor.cpp \
    transactions_manager.cpp

HEADERS += \
    account_edit.h \
    account_widget.h \
    add_account_window.h \
    add_category_window.h \
    add_inc_dec.h \
    add_transfer_window.h \
    binary_loader.h \
    domain.h \
    first_window.h \
    mainwindow.h \
    model.h \
    model_representation.h \
    serialization.h \
    transaction_editor.h \
    transactions_manager.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
