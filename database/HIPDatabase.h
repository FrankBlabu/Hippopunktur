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
#include <QVector2D>

class QDomDocument;

namespace HIP {
  namespace Database {

    /*
     * Object describing the a single position of a point
     */
    class Position : public QObject
    {
    public:
      Position ();
      Position (const Position& toCopy);
      virtual ~Position ();

      const QString& getImage () const { return _image; }
      void setImage (const QString& image);

      const QVector2D& getCoordinate () const { return _coordinate; }
      void setCoordinate (const QVector2D& coordinate);

      Position& operator= (const Position& toCopy);

    private:
      QString _image;
      QVector2D _coordinate;
    };

    /*
     * Object keeping the data of a single point
     */
    class Point : public QObject
    {
    public:
      Point ();
      Point (const Point& toCopy);
      virtual ~Point ();

      const QString& getId () const { return _id; }
      void setId (const QString& id);

      const QString& getDescription () const { return _description; }
      void setDescription (const QString& description);

      const QList<QString>& getTags () const { return _tags; }
      void setTags (const QList<QString>& tags);

      const QList<Position>& getPositions () const { return _positions; }
      void setPositions (const QList<Position>& positions);

      bool getSelected () const { return _selected; }
      void setSelected (bool state);

      Point& operator= (const Point& toCopy);

    private:
      QString _id;
      QString _description;
      QList<QString> _tags;
      QList<Position> _positions;

      bool _selected;
    };


    /*
     * Object representing a single image
     */
    class Image : public QObject
    {
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
    public:
      Database (const QString& path);
      virtual ~Database ();

      const QList<Point>& getPoints () const { return _points; }
      const QList<QString>& getTags () const { return _tags; }
      const QList<Image>& getImages () const { return _images; }

      void setPoint (const QString& id, const Point& point);
      void setSelected (const QString& id, bool selected);

    private:
      void computeTags ();
      int findIndex (const QString& id) const;

    private:
      QList<Point> _points;
      QList<QString> _tags;
      QList<Image> _images;
    };
  }
}

Q_DECLARE_METATYPE (HIP::Database::Point);
Q_DECLARE_METATYPE (HIP::Database::Image);

#endif
