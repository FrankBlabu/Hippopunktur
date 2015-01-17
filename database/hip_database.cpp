/*
 * hip_database.cpp - Database keeping all the medical data
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPDatabase.h"
#include "core/HIPException.h"

#include <QDebug>
#include <QDomDocument>
#include <QFile>

namespace HIP {
  namespace Database {

    //#**********************************************************************
    // Local data
    //#**********************************************************************

    namespace {

      namespace Tags
      {
        static const char* const DATABASE    = "database";
        static const char* const POINTS      = "points";
        static const char* const POINT       = "point";
        static const char* const TAG         = "tag";
        static const char* const POSITIONS   = "positions";
        static const char* const POSITION    = "position";
        static const char* const DESCRIPTION = "description";
        static const char* const IMAGES      = "images";
        static const char* const IMAGE       = "image";
      };

      namespace Attributes
      {
        static const char* const ID    = "id";
        static const char* const NAME  = "name";
        static const char* const X     = "x";
        static const char* const Y     = "y";
        static const char* const TITLE = "title";
        static const char* const PATH  = "path";
        static const char* const IMAGE = "image";
      }

    }

    //#**********************************************************************
    // CLASS HIP::Database::PointComparator
    //#**********************************************************************

    namespace {

      /*! Comparator class for point sorting */
      class PointComparator
      {
      public:
        inline PointComparator () {}
        inline bool operator () (const Point& p1, const Point& p2) const
        {
          return splitId (p1) < splitId (p2);
        }

      private:
        inline QPair<QString, QString> splitId (const Point& p) const
        {
          QString id = p.getId ();

          int index = id.length () - 1;
          while (index > 0 && id[index].isNumber ())
            --index;

          index = qMax (index, 0);

          return qMakePair (id.left (index), id.mid (index));
        }
      };

    }

    //#**********************************************************************
    // CLASS HIP::Database::Position
    //#**********************************************************************

    Position::Position ()
      : _image      (),
        _coordinate ()
    {
    }

    Position::Position (const Position& toCopy)
      : _image      (toCopy._image),
        _coordinate (toCopy._coordinate)
    {
    }

    Position::~Position ()
    {
    }

    void Position::setImage (const QString& image)
    {
      _image = image;
    }

    void Position::setCoordinate (const QVector2D& coordinate)
    {
      _coordinate = coordinate;
    }

    Position& Position::operator= (const Position& toCopy)
    {
      _image = toCopy._image;
      _coordinate = toCopy._coordinate;

      return *this;
    }


    //#**********************************************************************
    // CLASS HIP::Database::Point
    //#**********************************************************************

    /*! Constructor */
    Point::Point ()
      : _id          (),
        _description (),
        _tags        (),
        _positions   (),
        _selected    (false)
    {
    }

    /*! Copy constructor */
    Point::Point (const Point& toCopy)
      : QObject (),
        _id          (toCopy._id),
        _description (toCopy._description),
        _tags        (toCopy._tags),
        _positions   (toCopy._positions),
        _selected    (toCopy._selected)
    {
    }

    /*! Destructor */
    Point::~Point ()
    {
    }

    void Point::setId (const QString& id)
    {
      _id = id;
    }

    void Point::setDescription (const QString& description)
    {
      _description = description;
    }

    void Point::setTags (const QList<QString>& tags)
    {
      _tags = tags;
    }

    void Point::setPositions (const QList<Position>& positions)
    {
      _positions = positions;
    }

    void Point::setSelected (bool state)
    {
      _selected = state;
    }

    Point& Point::operator= (const Point& toCopy)
    {
      _id = toCopy._id;
      _description = toCopy._description;
      _tags = toCopy._tags;
      _positions = toCopy._positions;
      _selected = toCopy._selected;

      return *this;
    }


    //#**********************************************************************
    // CLASS HIP::Database::Image
    //#**********************************************************************

    Image::Image ()
      : _id    (),
        _title (),
        _path  ()
    {
    }

    Image::Image (const Image& toCopy)
      : _id    (toCopy._id),
        _title (toCopy._title),
        _path  (toCopy._path)
    {
    }

    Image::~Image ()
    {
    }

    void Image::setId (const QString& id)
    {
      _id = id;
    }

    void Image::setTitle (const QString& title)
    {
      _title = title;
    }

    void Image::setPath (const QString& path)
    {
      _path = path;
    }

    Image& Image::operator= (const Image& toCopy)
    {
      _id    = toCopy._id;
      _title = toCopy._title;
      _path  = toCopy._path;

      return *this;
    }


    //#**********************************************************************
    // CLASS HIP::Database::Database
    //#**********************************************************************

    /*! Constructor */
    Database::Database (const QString& path)
    {
      QFile file (path);

      QByteArray data;
      if (file.open (QFile::ReadOnly))
        {
          data = file.readAll ();
          file.close ();
        }
      else
        throw Exception (tr ("Unable to open input file %1").arg (path));

      QDomDocument doc;

      QString error_message;
      int error_line = -1;
      int error_column = 1;
      if (doc.setContent (data, &error_message, &error_line, &error_column))
        {
          if (doc.documentElement ().tagName () != Tags::DATABASE)
            throw Exception (tr ("Illegal points database format"));

          for ( QDomNode top_n = doc.documentElement ().firstChild (); !top_n.isNull ();
                top_n = top_n.nextSibling () )
            {
              QDomElement top_e = top_n.toElement ();

              //
              // Load points database
              //
              if (top_e.tagName () == Tags::POINTS)
                {
                  for ( QDomNode point_n = top_e.firstChild (); !point_n.isNull ();
                        point_n = point_n.nextSibling () )
                    {
                      QDomElement point_e = point_n.toElement ();
                      if (point_e.tagName () != Tags::POINT)
                        throw Exception (tr ("Point element expected, but got %1").arg (point_e.tagName ()));
                      if (!point_e.hasAttribute (Attributes::ID))
                        throw Exception (tr ("Point entry does not have an id"));

                      Point point;
                      point.setId (point_e.attribute (Attributes::ID));

                      QList<QString> tags;

                      QDomNodeList tags_l = point_e.elementsByTagName (Tags::TAG);
                      for (int i=0; i < tags_l.count (); ++i)
                        {
                          QDomElement tag_e = tags_l.at (i).toElement ();
                          if (!tag_e.hasAttribute (Attributes::NAME))
                            throw Exception (tr ("Tag entry does not have an name"));

                          tags.push_back (tag_e.attribute (Attributes::NAME));
                        }

                      QDomNodeList positions_l = point_e.elementsByTagName (Tags::POSITION);
                      if (positions_l.isEmpty ())
                        throw Exception (tr ("Point entry does not have a position"));

                      for (int i=0; i < positions_l.count (); ++i)
                        {
                          QDomElement position_e = positions_l.at (i).toElement ();
                        }

                      point.setTags (tags);

                      QDomElement description_e = point_e.namedItem (Tags::DESCRIPTION).toElement ();
                      if (description_e.isNull ())
                        throw Exception (tr ("Point entry does not have a description"));
                      if (!description_e.firstChild ().isCharacterData ())
                        throw Exception (tr ("Character data expected for point description"));

                      point.setDescription (description_e.firstChild ().toCharacterData ().data ());

                      _points.push_back (point);
                    }
                }

              //
              // Load image database
              //
              else if (top_e.tagName () == Tags::IMAGES)
                {
                  for ( QDomNode image_n = top_e.firstChild (); !image_n.isNull ();
                        image_n = image_n.nextSibling () )
                    {
                      QDomElement image_e = image_n.toElement ();
                      if (image_e.tagName () != Tags::IMAGE)
                        throw Exception (tr ("Image element expected, but got %1").arg (image_e.tagName ()));
                      if (!image_e.hasAttribute (Attributes::ID))
                        throw Exception (tr ("Image entry does not have an id"));
                      if (!image_e.hasAttribute (Attributes::TITLE))
                        throw Exception (tr ("Image entry does not have a title"));
                      if (!image_e.hasAttribute (Attributes::PATH))
                        throw Exception (tr ("Image entry does not have a path"));

                      Image image;
                      image.setId (image_e.attribute (Attributes::ID));
                      image.setTitle (image_e.attribute (Attributes::TITLE));
                      image.setPath (image_e.attribute (Attributes::PATH));

                      _images.append (image);
                    }
                }
            }
        }
      else
        throw Exception (tr ("Error parsing points database in line %1: %2").arg (error_line).arg (error_message));

      std::sort (_points.begin (), _points.end (), PointComparator ());

      computeTags ();
    }

    /*! Destructor */
    Database::~Database ()
    {
    }

    /*! Set point value */
    void Database::setPoint (const QString& id, const Point& point)
    {
      int index = findIndex (id);

      Q_ASSERT (index >= 0 && index < _points.size ());
      _points[index] = point;

      computeTags ();
    }

    /*! Set point selection status */
    void Database::setSelected (const QString &id, bool selected)
    {
      int index = findIndex (id);

      Q_ASSERT (index >= 0 && index < _points.size ());
      _points[index].setSelected (selected);
    }

    /*! Compute list of all existing tags */
    void Database::computeTags ()
    {
      QSet<QString> tags;
      foreach (const Point& point, _points)
        tags.unite (point.getTags ().toSet ());

      _tags = tags.toList ();

      std::sort (_tags.begin (), _tags.end ());
    }

    /*! Find index of the point matching a given id */
    int Database::findIndex (const QString &id) const
    {
      int index = -1;

      for (int i=0; i < _points.size () && index == -1; ++i)
        if (_points[i].getId () == id)
          index = i;

      return index;
    }

  }
}

