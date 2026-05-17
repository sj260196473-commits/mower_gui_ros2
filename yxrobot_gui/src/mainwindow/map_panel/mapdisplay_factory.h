#ifndef MAPDISPLAY_FACTORY_H
#define MAPDISPLAY_FACTORY_H

#include <QObject>

class MapDisplayFactory : public QObject
{
    Q_OBJECT
public:
    static MapDisplayFactory *Instance(){
        static MapDisplayFactory *factory = new MapDisplayFactory();
        return factory;
    }
    MapDisplayFactory();
};

#endif // MAPDISPLAY_FACTORY_H
