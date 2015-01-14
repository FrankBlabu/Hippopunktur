/*
 * HIPDatabase.h - Database keeping all the medical data
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPDatabase_h__
#define __HIPDatabase_h__

#include <QObject>
#include <QString>
#include <QList>
#include <QFile>

class QDomDocument;

namespace HIP {
  namespace Database {

    /*
     * Object keeping the data of a single point
     */
    class Point : public QObject
    {
    public:
      Point (const QString& id, const QString& description, const QList<QString>& tags);
      Point (const Point& toCopy);
      virtual ~Point ();

      const QString& getId () const { return _id; }
      const QString& getDescription () const { return _description; }
      const QList<QString>& getTags () const { return _tags; }

    private:
      QString _id;
      QString _description;
      QList<QString> _tags;
    };


    /*
     * Point keeping database
     */
    class Database : public QObject
    {
    public:
      Database (const QString& path);
      virtual ~Database ();

      const QList<Point>& getPoints () const;

    private:
      QList<Point> _points;
    };
  }
}

#endif
