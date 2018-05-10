# Qt Modules
QT += core sql
QT -= gui

# Build Enviroment
CONFIG += c++17
DEFINES += QT_DEPRECATED_WARNINGS # emit warnings if you use any feature of Qt which as been marked deprecated
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000 # disables all the APIs deprecated before Qt 6.0.0

# Target
TARGET = SqlLinq
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app

# Sources
SOURCES += src/main.cpp \
           src/sqlinq.cpp

# Headers
HEADERS += src/sqlinq.h
