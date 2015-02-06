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

    /*!
     * Single view of the object consisting of multiple object groups
     */
    class View
    {
    public:
      View ();
      ~View ();

      const QString& getName () const          { return _name; }
      const QList<QString>& getGroups () const { return _groups; }

      void setName (const QString& name)   { _name = name; }
      void addGroup (const QString& group) { _groups.push_back (group); }

    private:
      QString _name;
      QList<QString> _groups;
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
      struct Reason { enum Type_t { POINT, SELECTION, DATA, FILTER, VIEW }; };
      typedef Reason::Type_t Reason_t;

    public:
      Database ();
      virtual ~Database ();

      void load (const QString& data); // throws Exception

      const QList<Point>& getPoints () const { return _points; }
      const QList<QString>& getTags () const { return _tags; }
      const QList<View>& getViews () const   { return _views; }
      const GL::Data* getModel () const      { return _model; }

      const Point& getPoint (const QString& id) const;
      void setPoint (const Point& point);

      //
      // Point selection
      //
      void select (const QString& id);
      void deselect (const QString& id);
      void clearSelection ();

      //
      // Filter
      //
      const QString& getFilter () const;
      void setFilter (const QString& filter);

      //
      // Visible groups
      //
      const QString& getCurrentView () const;
      void setCurrentView (const QString& view);

      QString toXML () const;

      //
      // Signals
      //
      void emitViewChanged (const QVariant& data);

    signals:
      void databaseChanged (Reason_t reason, const QVariant& data);
      void viewChanged (const QVariant& data);

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
      QList<View> _views;

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
      QString _current_view;
    };
  }

  QDebug operator<< (QDebug stream, const Database::Point& point);
  QDebug operator<< (QDebug stream, const Database::Database::Reason_t reason);
}

Q_DECLARE_METATYPE (HIP::Database::Point)
Q_DECLARE_METATYPE (HIP::Database::Database::Reason_t)

#endif
