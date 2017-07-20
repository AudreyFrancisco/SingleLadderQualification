/****************************************************************************
** Meta object code from reading C++ file 'pbMainWindow.h'
**
** Created: Tue Jul 11 15:34:46 2017
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/pbMainWindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pbMainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_pbMainWindow[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      20,   14,   13,   13, 0x0a,
      36,   13,   13,   13, 0x2a,
      50,   47,   13,   13, 0x08,
      68,   47,   13,   13, 0x08,
      87,   47,   13,   13, 0x08,
     104,   47,   13,   13, 0x08,
     121,   47,   13,   13, 0x08,
     146,   13,   13,   13, 0x08,
     158,   13,   13,   13, 0x08,
     175,  169,   13,   13, 0x08,
     193,   13,   13,   13, 0x08,
     210,   13,   13,   13, 0x08,
     228,   13,   13,   13, 0x08,
     236,   13,   13,   13, 0x08,
     245,   13,   13,   13, 0x08,
     265,   13,   13,   13, 0x08,
     278,   13,   13,   13, 0x08,
     289,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_pbMainWindow[] = {
    "pbMainWindow\0\0fname\0fileOpen(char*)\0"
    "fileOpen()\0ch\0channelSetON(int)\0"
    "channelSetOFF(int)\0channelVset(int)\0"
    "channelIset(int)\0biasCheckBoxChanged(int)\0"
    "storeVset()\0VbiasSet()\0en,ch\0"
    "enVbias(bool,int)\0refreshMonitor()\0"
    "refreshSettings()\0allON()\0allOFF()\0"
    "saveConfiguration()\0fileSaveAs()\0"
    "fileSave()\0configure()\0"
};

const QMetaObject pbMainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_pbMainWindow,
      qt_meta_data_pbMainWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &pbMainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *pbMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *pbMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_pbMainWindow))
        return static_cast<void*>(const_cast< pbMainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int pbMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: fileOpen((*reinterpret_cast< char*(*)>(_a[1]))); break;
        case 1: fileOpen(); break;
        case 2: channelSetON((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: channelSetOFF((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: channelVset((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: channelIset((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: biasCheckBoxChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: storeVset(); break;
        case 8: VbiasSet(); break;
        case 9: enVbias((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 10: refreshMonitor(); break;
        case 11: refreshSettings(); break;
        case 12: allON(); break;
        case 13: allOFF(); break;
        case 14: saveConfiguration(); break;
        case 15: fileSaveAs(); break;
        case 16: fileSave(); break;
        case 17: configure(); break;
        default: ;
        }
        _id -= 18;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
