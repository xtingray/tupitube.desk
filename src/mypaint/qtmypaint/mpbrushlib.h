#ifndef MPBRUSHLIB_H
#define MPBRUSHLIB_H

#include <QMap>
#include <QString>

class MPBrushLib
{
public:
    MPBrushLib(const QString& brushLibPath);

    void selectBrush(QString brushName);

protected:
    QMap<QString, QStringList> m_brushLib;
    const QString              m_brushesPath;
};

#endif // MPBRUSHLIB_H
