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

    /*!
     * Object keeping the data of a single point
     *
     * A point resembles a complete accupuncture point including the location,
     * description, visualization and state (selection, ...)
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

      const QString& getId () const;
      void setId (const QString& id);

      const QString& getDescription () const;
      void setDescription (const QString& description);

      const QList<QString>& getTags () const;
      void setTags (const QList<QString>& tags);

      const QVector3D& getPosition () const;
      void setPosition (const QVector3D& position);

      const QColor& getColor () const;
      void setColor (const QColor& color);

      bool getSelected () const;
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
     *
     * A view is the semantic grouping of multiple object groups. It has a
     * meaning to the user like 'front', 'legs', ...
     */
    class View
    {
    public:
      View ();
      ~View ();

      const QString& getName () const;
      const QList<QString>& getGroups () const;

      void setName (const QString& name);
      void addGroup (const QString& group);

    private:
      QString _name;
      QList<QString> _groups;
    };

    /*!
     * Database keeping all relevant data structures and the related states
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

      const QList<Point>& getPoints () const;
      const QList<QString>& getTags () const;
      const QList<View>& getViews () const;
      const GL::Data* getModel () const;

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
