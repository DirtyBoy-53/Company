/****************************************************************************
** Meta object code from reading C++ file 'documentbase.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/canvas/documentbase.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'documentbase.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DocumentBase_t {
    QByteArrayData data[17];
    char stringdata0[175];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DocumentBase_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DocumentBase_t qt_meta_stringdata_DocumentBase = {
    {
QT_MOC_LITERAL(0, 0, 12), // "DocumentBase"
QT_MOC_LITERAL(1, 13, 19), // "currentShapeChanged"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 9), // "shapeName"
QT_MOC_LITERAL(4, 44, 11), // "sigAddShape"
QT_MOC_LITERAL(5, 56, 8), // "setScale"
QT_MOC_LITERAL(6, 65, 8), // "newScale"
QT_MOC_LITERAL(7, 74, 10), // "changeTask"
QT_MOC_LITERAL(8, 85, 11), // "task_mode_e"
QT_MOC_LITERAL(9, 97, 4), // "task"
QT_MOC_LITERAL(10, 102, 16), // "changeOperatMode"
QT_MOC_LITERAL(11, 119, 13), // "operat_mode_e"
QT_MOC_LITERAL(12, 133, 3), // "opt"
QT_MOC_LITERAL(13, 137, 14), // "changeDrawMode"
QT_MOC_LITERAL(14, 152, 11), // "draw_mode_e"
QT_MOC_LITERAL(15, 164, 4), // "draw"
QT_MOC_LITERAL(16, 169, 5) // "clean"

    },
    "DocumentBase\0currentShapeChanged\0\0"
    "shapeName\0sigAddShape\0setScale\0newScale\0"
    "changeTask\0task_mode_e\0task\0"
    "changeOperatMode\0operat_mode_e\0opt\0"
    "changeDrawMode\0draw_mode_e\0draw\0clean"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DocumentBase[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,
       4,    0,   52,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   53,    2, 0x0a /* Public */,
       7,    1,   56,    2, 0x0a /* Public */,
      10,    1,   59,    2, 0x0a /* Public */,
      13,    1,   62,    2, 0x0a /* Public */,
      16,    0,   65,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QReal,    6,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void,

       0        // eod
};

void DocumentBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DocumentBase *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->currentShapeChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->sigAddShape(); break;
        case 2: _t->setScale((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 3: _t->changeTask((*reinterpret_cast< task_mode_e(*)>(_a[1]))); break;
        case 4: _t->changeOperatMode((*reinterpret_cast< operat_mode_e(*)>(_a[1]))); break;
        case 5: _t->changeDrawMode((*reinterpret_cast< draw_mode_e(*)>(_a[1]))); break;
        case 6: _t->clean(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DocumentBase::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DocumentBase::currentShapeChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DocumentBase::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DocumentBase::sigAddShape)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DocumentBase::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_DocumentBase.data,
    qt_meta_data_DocumentBase,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DocumentBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DocumentBase::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DocumentBase.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DocumentBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void DocumentBase::currentShapeChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DocumentBase::sigAddShape()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
