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
#include <QPointF>

class QDomDocument;

namespace HIP {
  namespace Database {

    /*
     * Object describing the a single position of a point
     */
    class Position : public QObject
    {
      Q_OBJECT

      Q_PROPERTY (QString image READ getImage WRITE setImage)
      Q_PROPERTY (QPointF coordinate READ getCoordinate WRITE setCoordinate)

    public:
      Position ();
      Position (const Position& toCopy);
      virtual ~Position ();

      const QString& getImage () const { return _image; }
      void setImage (const QString& image);

      const QPointF& getCoordinate () const { return _coordinate; }
      void setCoordinate (const QPointF& coordinate);

      Position& operator= (const Position& toCopy);

    private:
      QString _image;
      QPointF _coordinate;
    };

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

      const QList<Position>& getPositions () const { return _positions; }
      void setPositions (const QList<Position>& positions);

      const QColor& getColor () const { return _color; }
      void setColor (const QColor& color);

      bool getSelected () const { return _selected; }
      void setSelected (bool state);

      Point& operator= (const Point& toCopy);

    private:
      QString _id;
      QString _description;
      QList<QString> _tags;
      QList<Position> _positions;
      QColor _color;

      bool _selected;
    };


    /*
     * Object representing a single image
     */
    class Image : public QObject
    {
      Q_OBJECT

      Q_PROPERTY (QString id READ getId WRITE setId)
      Q_PROPERTY (QString title READ getTitle WRITE setTitle)
      Q_PROPERTY (QString path READ getPath WRITE setPath)

    public:
      Image ();
      Image (const Image& toCopy);
      virtual ~Image ();

      const QString& getId () const { return _id; }
      void setId (const QString& id);

      const QString& getTitle () const { return _title; }
      void setTitle (const QString& title);

      const QString& getPath () const { return _path; }
      void setPath (const QString& path);

      Image& operator= (const Image& toCopy);

    private:
      QString _id;
      QString _title;
      QString _path;
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
      struct Reason { enum Type_t { POINT, SELECTION, DATA, FILTER, VISIBLE_IMAGE }; };
      typedef Reason::Type_t Reason_t;

    public:
      Database ();
      virtual ~Database ();

      void load (const QString& data); // throws Exception

      const QList<Point>& getPoints () const { return _points; }
      const QList<QString>& getTags () const { return _tags; }
      const QList<Image>& getImages () const { return _images; }

      const Point& getPoint (const QString& id) const;
      void setPoint (const Point& point);

      const Image& getImage (const QString& id) const;

      //
      // Point selection
      //
      void select (const QString& id);
      void deselect (const QString& id);
      void clearSelection ();

      const QString& getVisibleImage () const;
      void setVisibleImage (const QString& id);

      const QString& getFilter () const;
      void setFilter (const QString& filter);

      QString toXML () const;

    signals:
      void databaseChanged (Reason_t reason, const QString& id);

    private:
      void computeTags ();
      void computeIndices ();

      int findIndex (const QString& id) const;

    private:
      QString _name;

      QList<Point> _points;
      QList<QString> _tags;
      QList<Image> _images;

      typedef QMap<QString, int> PointIndexMap;
      PointIndexMap _point_indices;

      QString _visible_image;
      QString _filter;
    };
  }

  QDebug operator<< (QDebug stream, const Database::Position& position);
  QDebug operator<< (QDebug stream, const Database::Point& point);
  QDebug operator<< (QDebug stream, const Database::Database::Reason_t reason);
}

Q_DECLARE_METATYPE (HIP::Database::Position)
Q_DECLARE_METATYPE (HIP::Database::Point)
Q_DECLARE_METATYPE (HIP::Database::Image)
Q_DECLARE_METATYPE (HIP::Database::Database::Reason_t)

#endif
