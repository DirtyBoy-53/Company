/****************************************************************************
** Meta object code from reading C++ file 'labelmanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../src/controllers/labelmanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'labelmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LabelManager_t {
    QByteArrayData data[18];
    char stringdata0[172];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LabelManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LabelManager_t qt_meta_stringdata_LabelManager = {
    {
QT_MOC_LITERAL(0, 0, 12), // "LabelManager"
QT_MOC_LITERAL(1, 13, 12), // "labelChanged"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 12), // "labelRemoved"
QT_MOC_LITERAL(4, 40, 5), // "label"
QT_MOC_LITERAL(5, 46, 10), // "labelAdded"
QT_MOC_LITERAL(6, 57, 5), // "color"
QT_MOC_LITERAL(7, 63, 8), // "visibile"
QT_MOC_LITERAL(8, 72, 2), // "id"
QT_MOC_LITERAL(9, 75, 14), // "visibelChanged"
QT_MOC_LITERAL(10, 90, 7), // "visible"
QT_MOC_LITERAL(11, 98, 12), // "colorChanged"
QT_MOC_LITERAL(12, 111, 10), // "allCleared"
QT_MOC_LITERAL(13, 122, 8), // "addLabel"
QT_MOC_LITERAL(14, 131, 11), // "removeLabel"
QT_MOC_LITERAL(15, 143, 8), // "setColor"
QT_MOC_LITERAL(16, 152, 10), // "setVisible"
QT_MOC_LITERAL(17, 163, 8) // "allClear"

    },
    "LabelManager\0labelChanged\0\0labelRemoved\0"
    "label\0labelAdded\0color\0visibile\0id\0"
    "visibelChanged\0visible\0colorChanged\0"
    "allCleared\0addLabel\0removeLabel\0"
    "setColor\0setVisible\0allClear"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LabelManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x06 /* Public */,
       3,    1,   75,    2, 0x06 /* Public */,
       5,    4,   78,    2, 0x06 /* Public */,
       9,    2,   87,    2, 0x06 /* Public */,
      11,    2,   92,    2, 0x06 /* Public */,
      12,    0,   97,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    4,   98,    2, 0x0a /* Public */,
      13,    3,  107,    2, 0x2a /* Public | MethodCloned */,
      14,    1,  114,    2, 0x0a /* Public */,
      15,    2,  117,    2, 0x0a /* Public */,
      16,    2,  122,    2, 0x0a /* Public */,
      17,    0,  127,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void, QMetaType::QString, QMetaType::QColor, QMetaType::Bool, QMetaType::Int,    4,    6,    7,    8,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    4,   10,
    QMetaType::Void, QMetaType::QString, QMetaType::QColor,    4,    6,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QColor, QMetaType::Bool, QMetaType::Int,    4,    6,   10,    8,
    QMetaType::Void, QMetaType::QString, QMetaType::QColor, QMetaType::Bool,    4,    6,   10,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void, QMetaType::QString, QMetaType::QColor,    4,    6,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    4,   10,
    QMetaType::Void,

       0        // eod
};

void LabelManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LabelManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->labelChanged(); break;
        case 1: _t->labelRemoved((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->labelAdded((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QColor(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 3: _t->visibelChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 4: _t->colorChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QColor(*)>(_a[2]))); break;
        case 5: _t->allCleared(); break;
        case 6: _t->addLabel((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QColor(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 7: _t->addLabel((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QColor(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 8: _t->removeLabel((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 9: _t->setColor((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QColor(*)>(_a[2]))); break;
        case 10: _t->setVisible((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 11: _t->allClear(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LabelManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LabelManager::labelChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LabelManager::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LabelManager::labelRemoved)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LabelManager::*)(QString , QColor , bool , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LabelManager::labelAdded)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LabelManager::*)(QString , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LabelManager::visibelChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LabelManager::*)(QString , QColor );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LabelManager::colorChanged)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (LabelManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&LabelManager::allCleared)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject LabelManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_LabelManager.data,
    qt_meta_data_LabelManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *LabelManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LabelManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LabelManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int LabelManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void LabelManager::labelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void LabelManager::labelRemoved(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void LabelManager::labelAdded(QString _t1, QColor _t2, bool _t3, int _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void LabelManager::visibelChanged(QString _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void LabelManager::colorChanged(QString _t1, QColor _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void LabelManager::allCleared()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
