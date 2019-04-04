QT += testlib
QT -= gui

CONFIG += c++14

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_thread_safe.cpp

HEADERS += \
    ../../include/thread_safe_ptr.h
