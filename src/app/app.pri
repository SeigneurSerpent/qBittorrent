INCLUDEPATH += $$PWD

usesystemqtsingleapplication {
    nogui {
        CONFIG += qtsinglecoreapplication
    } else {
        CONFIG += qtsingleapplication
    }
} else {
    nogui {
        include(qtsingleapplication/qtsinglecoreapplication.pri)
    } else {
        include(qtsingleapplication/qtsingleapplication.pri)
    }
}

HEADERS += $$PWD/application.h
SOURCES += $$PWD/application.cpp

unix: HEADERS += $$PWD/stacktrace.h
strace_win {
    HEADERS += $$PWD/stacktrace_win.h
    !nogui {
        HEADERS += $$PWD/stacktrace_win_dlg.h \
                   $$PWD/stacktrace_win_dlg_misc.h
        FORMS += $$PWD/stacktrace_win_dlg.ui
        SOURCES += $$PWD/stacktrace_win.cpp \
                   $$PWD/stacktrace_win_dlg.cpp \
                   $$PWD/stacktrace_win_dlg_misc.cpp
    }
}

SOURCES += $$PWD/main.cpp

# upgrade code
HEADERS += $$PWD/upgrade.h
