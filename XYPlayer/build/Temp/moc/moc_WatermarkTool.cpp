/****************************************************************************
** Meta object code from reading C++ file 'WatermarkTool.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../Src/Protocol/UI/WatermarkTool.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WatermarkTool.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_WatermarkTool_t {
    QByteArrayData data[8];
    char stringdata0[122];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WatermarkTool_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WatermarkTool_t qt_meta_stringdata_WatermarkTool = {
    {
QT_MOC_LITERAL(0, 0, 13), // "WatermarkTool"
QT_MOC_LITERAL(1, 14, 12), // "sigWatermark"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 27), // "QList<CustomWatermarkInfo>&"
QT_MOC_LITERAL(4, 56, 4), // "list"
QT_MOC_LITERAL(5, 61, 17), // "on_btnAdd_clicked"
QT_MOC_LITERAL(6, 79, 20), // "on_btnDelete_clicked"
QT_MOC_LITERAL(7, 100, 21) // "on_btnConfirm_clicked"

    },
    "WatermarkTool\0sigWatermark\0\0"
    "QList<CustomWatermarkInfo>&\0list\0"
    "on_btnAdd_clicked\0on_btnDelete_clicked\0"
    "on_btnConfirm_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WatermarkTool[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   37,    2, 0x08 /* Private */,
       6,    0,   38,    2, 0x08 /* Private */,
       7,    0,   39,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void WatermarkTool::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<WatermarkTool *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sigWatermark((*reinterpret_cast< QList<CustomWatermarkInfo>(*)>(_a[1]))); break;
        case 1: _t->on_btnAdd_clicked(); break;
        case 2: _t->on_btnDelete_clicked(); break;
        case 3: _t->on_btnConfirm_clicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (WatermarkTool::*)(QList<CustomWatermarkInfo> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&WatermarkTool::sigWatermark)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject WatermarkTool::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_WatermarkTool.data,
    qt_meta_data_WatermarkTool,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *WatermarkTool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WatermarkTool::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_WatermarkTool.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int WatermarkTool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void WatermarkTool::sigWatermark(QList<CustomWatermarkInfo> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
