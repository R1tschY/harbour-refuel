#ifndef REQUEST_H
#define REQUEST_H

#include <QObject>

struct Station {
    QString id;
    QString name;
    QString brand;
    QString address;
    float latitude;
    float longitude;
    float distance;
    bool isOpen;
    float price;
};

class Request : public QObject
{
    Q_OBJECT

public:
    explicit Request(QObject *parent = nullptr);

    enum class Fuel {
        SuperE5,
        SuperE10,
        Diesel,
        All
    };
    Q_ENUM(Fuel)

    enum class Sorting {
        Price,
        Distance
    };
    Q_ENUM(Sorting)

signals:

};

#endif // REQUEST_H
