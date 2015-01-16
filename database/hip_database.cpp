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
        static const char* const TAG         = "tag";
        static const char* const POSITION    = "position";
        static const char* const DESCRIPTION = "description";
      };

      namespace Attributes
      {
        static const char* const ID   = "id";
        static const char* const NAME = "name";
        static const char* const X    = "x";
        static const char* const Y    = "y";
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
    // CLASS HIP::Database::Point
    //#**********************************************************************

    /*! Constructor */
    Point::Point (const QString& id, const QString& description, const QList<QString>& tags)
      : _id          (id),
        _description (description),
        _tags        (tags),
        _selected    (false)
    {
    }

    /*! Copy constructor */
    Point::Point (const Point& toCopy)
      : QObject (),
        _id          (toCopy._id),
        _description (toCopy._description),
        _tags        (toCopy._tags),
        _selected    (toCopy._selected)
    {
    }

    /*! Destructor */
    Point::~Point ()
    {
    }

    bool Point::getSelected () const
    {
      return _selected;
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
      _selected = toCopy._selected;

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

          for ( QDomNode points_n = doc.documentElement ().firstChild (); !points_n.isNull ();
                points_n = points_n.nextSibling () )
            {
              QDomElement points_e = points_n.toElement ();

              if (points_e.tagName () == Tags::POINTS)
                {
                  for ( QDomNode point_n = points_e.firstChild (); !point_n.isNull ();
                        point_n = point_n.nextSibling () )
                    {
                      QDomElement point_e = point_n.toElement ();
                      if (!point_e.hasAttribute (Attributes::ID))
                        throw Exception (tr ("Point entry does not have an id"));

                      QString id = point_e.attribute (Attributes::ID);
                      QList<QString> tags;

                      QDomNodeList tags_l = point_e.elementsByTagName (Tags::TAG);
                      for (int i=0; i < tags_l.count (); ++i)
                        {
                          QDomElement tag_e = tags_l.at (i).toElement ();
                          if (!tag_e.hasAttribute (Attributes::NAME))
                            throw Exception (tr ("Tag entry does not have an name"));

                          tags.push_back (tag_e.attribute (Attributes::NAME));
                        }

                      QDomElement position_e = point_e.namedItem (Tags::POSITION).toElement ();
                      if (position_e.isNull ())
                        throw Exception (tr ("Point entry does not have a position"));

                      QDomElement description_e = point_e.namedItem (Tags::DESCRIPTION).toElement ();
                      if (description_e.isNull ())
                        throw Exception (tr ("Point entry does not have a description"));
                      if (!description_e.firstChild ().isCharacterData ())
                        throw Exception (tr ("Character data expected for point description"));

                      QString description = description_e.firstChild ().toCharacterData ().data ();

                      _points.push_back (Point (id, description, tags));
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
    void Database::setPoint (int index, const Point& point)
    {
      Q_ASSERT (index >= 0 && index < _points.size ());
      _points[index] = point;

      computeTags ();
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

  }
}

