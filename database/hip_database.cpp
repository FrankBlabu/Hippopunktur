/*
 * hip_database.cpp - Database keeping all the medical data
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPDatabase.h"
#include "core/HIPException.h"

#include <QColor>
#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QXmlStreamWriter>

namespace HIP {

  //#************************************************************************
  // Debug operators
  //#************************************************************************

  QDebug operator<< (QDebug stream, const Database::Position& position)
  {
    stream << "Position (image=" << position.getImage ()
           << ", coordinate=" << position.getCoordinate ()
           << ")";

    return stream;
  }

  QDebug operator<< (QDebug stream, const Database::Point& point)
  {
    stream << "Point (id=" << point.getId ()
           << ", description=" << point.getDescription ()
           << ", tags=" << point.getTags ()
           << ", positions=" << point.getPositions ().count ()
           << ", color="  << point.getColor ()
           << ", selected=" << point.getSelected ()
           << ")";

    return stream;
  }

  QDebug operator<< (QDebug stream, Database::Database::Reason_t reason)
  {
    switch (reason)
      {
      case Database::Database::Reason::DATA:
        stream << "DATA";
        break;
      case Database::Database::Reason::SELECTION:
        stream << "SELECTION";
        break;
      case Database::Database::Reason::POINT:
        stream << "POINT";
        break;
      case Database::Database::Reason::FILTER:
        stream << "FILTER";
        break;
      case Database::Database::Reason::VISIBLE_IMAGE:
        stream << "VISIBLE_IMAGE";
        break;
      }

    return stream;
  }


  namespace Database {

    //#**********************************************************************
    // Local data
    //#**********************************************************************

    namespace {

      //
      // Current (written) XML file version
      //
      static const char* const XML_VERSION = "0.1";

      //
      // Tags used in the XML file
      //
      namespace Tags
      {
        static const char* const DATABASE    = "database";
        static const char* const MODEL       = "model";
        static const char* const FILE    = "file";
        static const char* const POINTS      = "points";
        static const char* const POINT       = "point";
        static const char* const TAGS        = "tags";
        static const char* const TAG         = "tag";
        static const char* const POSITIONS   = "positions";
        static const char* const POSITION    = "position";
        static const char* const DESCRIPTION = "description";
        static const char* const IMAGES      = "images";
        static const char* const IMAGE       = "image";
        static const char* const COLOR       = "color";
      };

      //
      // Attributes used in the XML file
      //
      namespace Attributes
      {
        static const char* const VERSION = "version";
        static const char* const ID      = "id";
        static const char* const NAME    = "name";
        static const char* const X       = "x";
        static const char* const Y       = "y";
        static const char* const TITLE   = "title";
        static const char* const PATH    = "path";
        static const char* const IMAGE   = "image";
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
        inline QPair<QString, int> splitId (const Point& p) const
        {
          QString id = p.getId ();

          int index = id.length () - 1;
          while (index > 0 && id[index].isNumber ())
            --index;

          bool ok = false;
          int count = id.mid (index + 1).toInt (&ok);

          return ok ? qMakePair (id.left (index + 1), count) : qMakePair (id, 0);
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

    void Position::setCoordinate (const QPointF& coordinate)
    {
      _coordinate = coordinate;
    }

    Position& Position::operator= (const Position& toCopy)
    {
      _image      = toCopy._image;
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
        _color       (),
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
        _color       (toCopy._color),
        _selected    (toCopy._selected)
    {
    }

    /*! Destructor */
    Point::~Point ()
    {
    }

    /*! Check if this is a valid point */
    bool Point::isValid () const
    {
      return !_id.isEmpty ();
    }

    /*! Check if the point matches the given tag */
    bool Point::matches (const QString& tag) const
    {
      bool match = false;

      if (tag.isEmpty ())
        match = true;
      else if (_id.startsWith (tag, Qt::CaseInsensitive))
        match = true;
      else
        {
          foreach (const QString& t, _tags)
            if (t.startsWith (tag, Qt::CaseInsensitive))
              match = true;
        }

      return match;
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

    void Point::setColor (const QColor& color)
    {
      _color = color;
    }

    void Point::setSelected (bool state)
    {
      _selected = state;
    }

    Point& Point::operator= (const Point& toCopy)
    {
      _id          = toCopy._id;
      _description = toCopy._description;
      _tags        = toCopy._tags;
      _positions   = toCopy._positions;
      _color       = toCopy._color;
      _selected    = toCopy._selected;

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
    Database::Database ()
      : _points        (),
        _tags          (),
        _images        (),
        _model         (),
        _point_indices (),
        _visible_image ()
    {
    }

    /*! Load XML based database */
    void Database::load (const QString& data)
    {
      QString database_name;
      QList<Point> database_points;
      QList<QString> database_tags;
      QList<Image> database_images;

      QDomDocument doc;

      QString error_message;
      int error_line = -1;
      int error_column = 1;
      if (doc.setContent (data, &error_message, &error_line, &error_column))
        {
          if (doc.documentElement ().tagName () != Tags::DATABASE)
            throwDOMException (doc.documentElement (), tr ("Illegal points database format"));

          if (!doc.documentElement ().hasAttribute (Attributes::NAME))
            throwDOMException (doc.documentElement (), tr ("Database name missing"));

          database_name = doc.documentElement ().attribute (Attributes::NAME);

          for ( QDomNode top_n = doc.documentElement ().firstChild (); !top_n.isNull ();
                top_n = top_n.nextSibling () )
            {
              QDomElement top_e = top_n.toElement ();

              //
              // Load model information
              //
              if (top_e.tagName () == Tags::MODEL)
                {
                  QDomElement file_e = top_e.namedItem (Tags::FILE).toElement ();
                  if (file_e.isNull ())
                    throwDOMException (file_e, tr ("Model entry must have a file name"));
                  if (!file_e.firstChild ().isCharacterData ())
                    throwDOMException (file_e, tr ("Character data expected for file name"));

                  _model = file_e.firstChild ().toCharacterData ().data ();
                }

              //
              // Load points database
              //
              else if (top_e.tagName () == Tags::POINTS)
                {
                  for ( QDomNode point_n = top_e.firstChild (); !point_n.isNull ();
                        point_n = point_n.nextSibling () )
                    {
                      //
                      // Element: Point
                      //
                      QDomElement point_e = point_n.toElement ();
                      if (point_e.tagName () != Tags::POINT)
                        throwDOMException (point_e, tr ("Point element expected, but got %1").arg (point_e.tagName ()));
                      if (!point_e.hasAttribute (Attributes::ID))
                        throwDOMException (point_e, tr ("Point entry does not have an id"));

                      Point point;
                      point.setId (point_e.attribute (Attributes::ID));

                      //
                      // Category: Point::Tags
                      //
                      QList<QString> tags;

                      QDomNodeList tags_l = point_e.elementsByTagName (Tags::TAG);
                      for (int i=0; i < tags_l.count (); ++i)
                        {
                          QDomElement tag_e = tags_l.at (i).toElement ();
                          if (!tag_e.hasAttribute (Attributes::NAME))
                            throwDOMException (tag_e, tr ("Tag entry does not have an name"));

                          tags.push_back (tag_e.attribute (Attributes::NAME));
                        }

                      point.setTags (tags);

                      //
                      // Category: Point::Positions
                      //
                      QList<Position> positions;

                      QDomNodeList positions_l = point_e.elementsByTagName (Tags::POSITION);
                      if (positions_l.isEmpty ())
                        throwDOMException (point_e, tr ("Point entry does not have a position"));

                      for (int i=0; i < positions_l.count (); ++i)
                        {
                          QDomElement position_e = positions_l.at (i).toElement ();
                          if (!position_e.hasAttribute (Attributes::IMAGE))
                            throwDOMException (position_e, tr ("Position must specify an image"));
                          if (!position_e.hasAttribute (Attributes::X))
                            throwDOMException (position_e, tr ("Position must specify an x coordinate"));
                          if (!position_e.hasAttribute (Attributes::Y))
                            throwDOMException (position_e, tr ("Position must specify a y coordinate"));

                          bool x_ok = true;
                          bool y_ok = true;

                          Position position;
                          position.setImage (position_e.attribute (Attributes::IMAGE));
                          position.setCoordinate (QPointF (position_e.attribute (Attributes::X).toDouble (&x_ok),
                                                           position_e.attribute (Attributes::Y).toDouble (&y_ok)));

                          if (!x_ok)
                            throwDOMException (position_e, tr ("X coordinate is not a number"));
                          if (!y_ok)
                            throwDOMException (position_e, tr ("Y coordinate is not a number"));

                          positions.push_back (position);
                        }

                      point.setPositions (positions);

                      //
                      // Attribute: Point::Description
                      //
                      QDomElement description_e = point_e.namedItem (Tags::DESCRIPTION).toElement ();
                      if (description_e.isNull ())
                        throwDOMException (description_e, tr ("Point entry does not have a description"));
                      if (!description_e.firstChild ().isCharacterData ())
                        throwDOMException (description_e, tr ("Character data expected for point description"));


                      point.setDescription (description_e.firstChild ().toCharacterData ().data ());

                      //
                      // Attribute: Point::Color
                      //
                      QDomElement color_e = point_e.namedItem (Tags::COLOR).toElement ();
                      if (color_e.isNull ())
                        throwDOMException (color_e, tr ("Point entry does not have a color"));
                      if (!color_e.firstChild ().isCharacterData ())
                        throwDOMException (color_e, tr ("Character data expected for point color"));

                      QString color_name = color_e.firstChild ().toCharacterData ().data ();
                      QColor color (color_name);
                      if (!color.isValid ())
                        throwDOMException (color_e, tr ("Invalid point color '%1'").arg (color_name));

                      point.setColor (color);

                      database_points.push_back (point);
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
                      if (image_e.tagName () == Tags::IMAGE)
                        {
                          if (!image_e.hasAttribute (Attributes::ID))
                            throwDOMException (image_e, tr ("Image entry does not have an id"));
                          if (!image_e.hasAttribute (Attributes::TITLE))
                            throwDOMException (image_e, tr ("Image entry does not have a title"));
                          if (!image_e.hasAttribute (Attributes::PATH))
                            throwDOMException (image_e, tr ("Image entry does not have a path"));

                          Image image;
                          image.setId (image_e.attribute (Attributes::ID));
                          image.setTitle (image_e.attribute (Attributes::TITLE));
                          image.setPath (image_e.attribute (Attributes::PATH));

                          database_images.append (image);
                        }
                    }
                }
            }
        }
      else
        throw Exception (tr ("Error parsing points database in line %1: %2").arg (error_line).arg (error_message));

      std::sort (database_points.begin (), database_points.end (), PointComparator ());

      if (database_images.isEmpty ())
        throw Exception (tr ("No images found in database."));

      //
      // At this point everything went OK, loaded data can be assigned
      //
      _name = database_name;
      _points = database_points;
      _tags = database_tags;
      _images = database_images;
      _visible_image = _images.front ().getId ();

      computeIndices ();
      computeTags ();

      emit databaseChanged (Reason::DATA, QVariant ());
    }

    /*! Destructor */
    Database::~Database ()
    {
    }

    /* Access point with the given id */
    const Point& Database::getPoint (const QString& id) const
    {
      if (!_point_indices.contains (id))
        qWarning () << "ERROR !";

      Q_ASSERT (_point_indices.contains (id));
      return _points.at (_point_indices.value (id));
    }

    /*! Set point value */
    void Database::setPoint (const Point& point)
    {
      int index = findIndex (point.getId ());

      Q_ASSERT (index >= 0 && index < _points.size () && "Adding points is not supported here.");

      bool selected = _points[index].getSelected ();
      _points[index] = point;
      _points[index].setSelected (selected);

      std::sort (_points.begin (), _points.end (), PointComparator ());

      computeIndices ();
      computeTags ();

      emit databaseChanged (Reason::DATA, QVariant ());
    }

    /*! Get image entry from database */
    const Image& Database::getImage (const QString& id) const
    {
      int index = -1;

      for (int i=0; i < _images.size () && index == -1; ++i)
        if (_images[i].getId () == id)
          index = i;

      Q_ASSERT (index >= 0);

      return _images[index];
    }

    /*! Set point selected */
    void Database::select (const QString& id)
    {
      int index = findIndex (id);
      Q_ASSERT (index >= 0 && index < _points.size ());

      Point& point = _points[index];

      if (!point.getSelected ())
        {
          point.setSelected (true);
          emit databaseChanged (Reason::SELECTION, qVariantFromValue (point.getId ()));
        }
    }

    /*! Set point deselected */
    void Database::deselect (const QString& id)
    {
      int index = findIndex (id);
      Q_ASSERT (index >= 0 && index < _points.size ());

      Point& point = _points[index];

      if (point.getSelected ())
        {
          point.setSelected (false);
          emit databaseChanged (Reason::SELECTION, qVariantFromValue (point.getId ()));
        }
    }

    /*! Return id of the currently visible image */
    const QString& Database::getVisibleImage () const
    {
      return _visible_image;
    }

    /*! Set visible image */
    void Database::setVisibleImage (const QString& id)
    {
      bool found = false;
      foreach (const Image& image, _images)
        if (image.getId () == id)
          found = true;

      Q_ASSERT (found && "Invalid image id");

      _visible_image = id;

      emit databaseChanged (Reason::VISIBLE_IMAGE, qVariantFromValue (_visible_image));
    }

    /*! Return the current filter configuration */
    const QString& Database::getFilter () const
    {
      return _filter;
    }

    /*! Set filter configuration */
    void Database::setFilter (const QString &filter)
    {
      _filter = filter;
      emit databaseChanged (Reason::FILTER, qVariantFromValue (_filter));
    }

    /*! Clear selection */
    void Database::clearSelection ()
    {
      for (int i=0; i < _points.size (); ++i)
        {
          Point& point = _points[i];

          if (point.getSelected ())
            {
              point.setSelected (false);
              emit databaseChanged (Reason::SELECTION, qVariantFromValue (point.getId ()));
            }
        }
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


    /*! Compute point index list */
    void Database::computeIndices ()
    {
      _point_indices.clear ();

      for (int i=0; i < _points.size (); ++i)
        {
          const Point& point = _points[i];
          _point_indices.insert (point.getId (), i);
        }
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

    /*! Generate XML representation of the database */
    QString Database::toXML () const
    {
      QString text;

      QXmlStreamWriter out (&text);
      out.setAutoFormatting (true);

      out.writeStartDocument ();
      out.writeStartElement (Tags::DATABASE);
      out.writeAttribute (Attributes::VERSION, QString (XML_VERSION));
      out.writeAttribute (Attributes::NAME, _name);

      out.writeComment (tr ("Model"));
      out.writeStartElement (Tags::MODEL);
      out.writeStartElement (Tags::FILE);
      out.writeCharacters (_model);
      out.writeEndElement ();
      out.writeEndElement ();

      out.writeComment (tr ("List of points"));

      {
        out.writeStartElement (Tags::POINTS);

        foreach (const Point& point, _points)
          {
            out.writeStartElement (Tags::POINT);
            out.writeAttribute (Attributes::ID, point.getId ());

            {
              out.writeStartElement (Tags::TAGS);

              foreach (const QString& tag, point.getTags ())
                {
                  out.writeStartElement (Tags::TAG);
                  out.writeAttribute (Attributes::NAME, tag);
                  out.writeEndElement ();
                }

              out.writeEndElement ();

              out.writeStartElement (Tags::POSITIONS);

              foreach (const Position& position, point.getPositions ())
                {
                  out.writeStartElement (Tags::POSITION);
                  out.writeAttribute (Attributes::IMAGE, position.getImage ());
                  out.writeAttribute (Attributes::X, QString::number (position.getCoordinate ().x ()));
                  out.writeAttribute (Attributes::Y, QString::number (position.getCoordinate ().y ()));
                  out.writeEndElement ();
                }

              out.writeEndElement ();

              out.writeStartElement (Tags::DESCRIPTION);
              out.writeCharacters (point.getDescription ());
              out.writeEndElement ();

              out.writeStartElement (Tags::COLOR);
              out.writeCharacters (point.getColor ().name ());
              out.writeEndElement ();
            }

            out.writeEndElement ();
          }

        out.writeEndElement ();
      }

      out.writeComment (tr ("List of images"));

      {
        out.writeStartElement (Tags::IMAGES);

        foreach (const Image& image, _images)
          {
            out.writeStartElement (Tags::IMAGE);
            out.writeAttribute (Attributes::ID, image.getId ());
            out.writeAttribute (Attributes::TITLE, image.getTitle ());
            out.writeAttribute (Attributes::PATH, image.getPath ());
            out.writeEndElement ();
          }

        out.writeEndElement ();
      }

      out.writeEndElement ();
      out.writeEndDocument ();

      return text;
    }

    /*
     * Throw exception with node related error information
     */
    void Database::throwDOMException (const QDomNode& node, const QString& message) const
    {
      if (node.lineNumber () >= 0)
        throw Exception (tr ("Error in line %1: %2")
                         .arg (node.lineNumber ())
                         .arg (message));
      else
        throw Exception (message);

    }


  }
}

