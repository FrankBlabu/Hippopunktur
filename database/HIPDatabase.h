/*
 * HIPDatabase.h - Database keeping all the medical data
 *
 * Frank Blankenburg, Jan. 2015
 */

#ifndef __HIPDatabase_h__
#define __HIPDatabase_h__

#include <QObject>
#include <QColor>
#include <QString>
#include <QList>
#include <QMap>
#include <QFile>
#include <QVector3D>

class QDomNode;

namespace HIP {

  namespace GL {
    class Data;
  }

  namespace Database {

    /*
     * Object keeping the data of a single point
     */
    class Point : public QObject
    {
      Q_OBJECT

      Q_PROPERTY (QString id READ getId WRITE setId)
      Q_PROPERTY (QString description READ getDescription WRITE setDescription ())
      Q_PROPERTY (QList<QString> tags READ getTags WRITE setTags)
      Q_PROPERTY (QColor color READ getColor WRITE setColor)
      Q_PROPERTY (bool selected READ getSelected WRITE setSelected)

    public:
      Point ();
      Point (const Point& toCopy);
      virtual ~Point ();

      bool isValid () const;
      bool matches (const QString& tag) const;

      const QString& getId () const { return _id; }
      void setId (const QString& id);

      const QString& getDescription () const { return _description; }
      void setDescription (const QString& description);

      const QList<QString>& getTags () const { return _tags; }
      void setTags (const QList<QString>& tags);

      const QVector3D& getPosition () const { return _position; }
      void setPosition (const QVector3D& position);

      const QColor& getColor () const { return _color; }
      void setColor (const QColor& color);

      bool getSelected () const { return _selected; }
      void setSelected (bool state);

      Point& operator= (const Point& toCopy);

    private:
      QString _id;
      QString _description;
      QList<QString> _tags;
      QVector3D _position;
      QColor _color;

      bool _selected;
    };

    /*
     * Database keeping all relevant data structures
     */
    class Database : public QObject
    {
      Q_OBJECT

    private:
      Database (const Database& toCopy) { Q_UNUSED (toCopy); }

    public:
      struct Reason { enum Type_t { POINT, SELECTION, DATA, FILTER }; };
      typedef Reason::Type_t Reason_t;

    public:
      Database ();
      virtual ~Database ();

      void load (const QString& data); // throws Exception

      const QList<Point>& getPoints () const { return _points; }
      const QList<QString>& getTags () const { return _tags; }
      const GL::Data* getModel () const      { return _model; }

      const Point& getPoint (const QString& id) const;
      void setPoint (const Point& point);

      //
      // Point selection
      //
      void select (const QString& id);
      void deselect (const QString& id);
      void clearSelection ();

      const QString& getFilter () const;
      void setFilter (const QString& filter);

      QString toXML () const;

    signals:
      void databaseChanged (Reason_t reason, const QVariant& data);

    private:
      void computeTags ();
      void computeIndices ();
      void throwDOMException (const QDomNode& node, const QString& message) const;

      int findIndex (const QString& id) const;

    private:
      //
      // Database data
      //
      QString _name;

      QList<Point> _points;
      QList<QString> _tags;

      GL::Data* _model;

      //
      // Database cached data
      //
      typedef QMap<QString, int> PointIndexMap;
      PointIndexMap _point_indices;

      //
      // Database state
      //
      QString _filter;
    };
  }

  QDebug operator<< (QDebug stream, const Database::Point& point);
  QDebug operator<< (QDebug stream, const Database::Database::Reason_t reason);
}

Q_DECLARE_METATYPE (HIP::Database::Point)
Q_DECLARE_METATYPE (HIP::Database::Database::Reason_t)

#endif
