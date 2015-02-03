/*
 * hip_gl_model.cpp - Class for loading and keeping a GL model
 *
 * Frank Blankenburg, Jan. 2014
 */

#include "HIPGLModel.h"
#include "core/HIPException.h"
#include "core/HIPTools.h"

#include <QDebug>
#include <QTextStream>

namespace HIP {

  //#************************************************************************
  // Debug
  //#************************************************************************

  QDebug& operator<< (QDebug& stream, const GL::Face& face)
  {
    stream << "f";

    foreach (const GL::Point& point, face.getPoints ())
      stream << " " << point;

    return stream;
  }

  QDebug& operator<< (QDebug& stream, const GL::Point& point)
  {
    stream << point.getVertexIndex () << "/" << point.getTextureIndex () << "/" << point.getNormalIndex ();
    return stream;
  }

  QDebug& operator<< (QDebug& stream, const GL::Group& group)
  {
    stream << "Group (name=" << group.getName ()
           << ", material=" << group.getMaterial ()
           << "," << group.getFaces ().size () << " faces)";
    return stream;
  }

  QDebug& operator<< (QDebug& stream, const GL::Material& material)
  {
    stream << "Material (name=" << material.getName ()
           << ", ka=" << material.getAmbient ()
           << ", kd=" << material.getDiffuse ()
           << ", ks=" << material.getSpecular ()
           << ", d=" << material.getDissolved ()
           << ", ns=" << material.getSpecularExponent ()
           << ", ni=" << material.getOpticalDensity ()
           << ", texture=" << material.getTexture ()
           << ")";
    return stream;
  }


  namespace GL {

    //#**********************************************************************
    // Local functions
    //#**********************************************************************

    namespace {

      /* Convert string into double value */
      qreal toReal (const QString& v) // throws Exception
      {
        bool ok = false;
        qreal d = v.toFloat (&ok);

        if (!ok)
          throw Exception (QObject::tr ("Double value expected"));

        return d;
      }

      /* Convert string into integer value */
      int toInt (const QString& v) // throws Exception
      {
        bool ok = false;
        int i = v.toInt (&ok);

        if (!ok)
          throw Exception (QObject::tr ("Double value expected"));

        return i;
      }

      /* Convert string tuple into 3d vector */
      QVector3D toVector3d (const QString& x, const QString& y, const QString& z) // throws Exception
      {
        return QVector3D (toReal (x), toReal (y), toReal (z));
      }

      /* Convert string tuple into 2d vector */
      QVector2D toVector2d (const QString& x, const QString& y) // throws Exception
      {
        return QVector2D (toReal (x), toReal (y));
      }

    }


    //#**********************************************************************
    // CLASS HIP::GL::Point
    //#**********************************************************************

    /*! Constructor */
    Point::Point (int vertex_index, int normal_index, int texture_index)
      : _vertex_index  (vertex_index),
        _normal_index  (normal_index),
        _texture_index (texture_index)
    {
    }

    /*! Comparison operator */
    bool Point::operator< (const Point& point) const
    {
      bool less = false;

      if (_vertex_index == point._vertex_index)
        {
          if (_normal_index < point._normal_index)
            less = _texture_index < point._texture_index;
          else
            less = _normal_index < point._normal_index;
        }
      else
        less = _vertex_index < point._vertex_index;

      return less;
    }


    //#**********************************************************************
    // CLASS HIP::GL::Face
    //#**********************************************************************

    /*! Constructor */
    Face::Face (const QList<Point>& points)
      : _points (points)
    {
    }

    /*! Destructor */
    Face::~Face ()
    {
    }

    /*! Set normal index of face points */
    void Face::setNormalIndex (int index)
    {
      for (int i=0; i < _points.size (); ++i)
        _points[i].setNormalIndex (index);
    }


    //#**********************************************************************
    // CLASS HIP::GL::Material
    //#**********************************************************************

    /*! Constructor */
    Material::Material ()
      : _name              (""),
        _ambient           (),
        _diffuse           (),
        _specular          (),
        _dissolved         (1.0),
        _specular_exponent (100),
        _optical_density   (1.5)
    {
    }

    /*! Destructor */
    Material::~Material ()
    {
    }


    //#**********************************************************************
    // CLASS HIP::GL::Group
    //#**********************************************************************

    /*! Constructor */
    Group::Group ()
      : _name     (),
        _material (),
        _faces    ()
    {
    }

    /*! Destructor */
    Group::~Group ()
    {
    }

    /*!
     * Set index of the normals in the given face
     *
     * \param face_index Index of the face to access
     * \param index      Normal index to set
     */
    void Group::setNormalIndex (int face_index, int index)
    {
      Q_ASSERT (face_index >= 0 && face_index < _faces.size ());
      _faces[face_index].setNormalIndex (index);
    }


    //#**********************************************************************
    // CLASS HIP::GL::Model
    //#**********************************************************************

    /*! Constructor */
    Model::Model (const QString& path)
      : _name         (),
        _vertices     (),
        _normals      (),
        _textures     (),
        _groups       (),
        _materials    (),
        _bounding_box ()
    {
      QString content = Tools::loadResource<QString> (path);
      QString material_library;

      QTextStream file (&content, QIODevice::ReadOnly);

      GroupPtr group (new Group ());

      for (QString line=file.readLine ().trimmed (); !line.isNull (); line=file.readLine ().trimmed ())
        {
          QTextStream in (&line, QIODevice::ReadOnly);

          QString tag;
          in >> tag;
          tag = tag.toLower ();

          //
          // Object name
          //
          if (tag == "o")
            {
              in >> _name;
            }

          //
          // Vertex
          //
          else if (tag == "v")
            {
              QString x, y, z;
              in >> x >> y >> z;
              _vertices.push_back (toVector3d (x, y, z));
            }

          //
          // Vertex normal
          //
          else if (tag == "vn")
            {
              QString x, y, z;
              in >> x >> y >> z;
              _normals.push_back (toVector3d (x, y, z));
            }

          //
          // Vertex texture
          //
          else if (tag == "vt")
            {
              QString x, y;
              in >> x >> y;
              _textures.push_back (toVector2d (x, y));
            }

          //
          // Face
          //
          else if (tag == "f")
            {
              QList<Point> points;

              for (in >> tag; !tag.isNull (); in >> tag)
                points.push_back (toPoint (tag));

              if (points.size () == 3)
                group->addFace (Face (points));
              else if (points.size () == 4)
                {
                  QList<Point> points1;
                  points1 << points[0] << points[1] << points[2];
                  group->addFace (Face (points1));

                  QList<Point> points2;
                  points2 << points[1] << points[2] << points[3];
                  group->addFace (Face (points2));
                }
              else
                throw Exception (QObject::tr ("Only triangular or rectangular faces supported."));
            }

          //
          // Group
          //
          else if (tag == "g")
            {
              QString name;
              in >> name;

              if (!group->getFaces ().isEmpty ())
                _groups.push_back (group);

              group = GroupPtr (new Group ());
              group->setName (name);
            }

          //
          // Used material
          //
          else if (tag == "usemtl")
            {
              QString material;
              in >> material;
              group->setMaterial (material);
            }

          //
          // Related material library
          //
          else if (tag == "mtllib")
            {
              in >> material_library;
            }
        }

      _groups.push_back (group);

      //
      // Normalize vertices
      //
      float max_length = 0.0f;

      foreach (const QVector3D& vertex, _vertices)
        if (vertex.length () > max_length)
          max_length = vertex.length ();

      for (int i=0; i < _vertices.size (); ++i)
        _vertices[i] /= max_length;

      //
      // Add missing normals
      //
      for (int i=0; i < _groups.size (); ++i)
        {
          GroupPtr group = _groups[i];

          for (int j=0; j < group->getFaces ().size (); ++j)
            {
              const Face& face = group->getFaces ()[j];

              Q_ASSERT (face.getPoints ().size () == 3);

              if (face.getPoints ()[0].getNormalIndex () == -1)
                {
                  Q_ASSERT (face.getPoints ()[1].getNormalIndex () == -1);
                  Q_ASSERT (face.getPoints ()[2].getNormalIndex () == -1);

                  QVector3D p0 = _vertices[face.getPoints ()[0].getVertexIndex ()];
                  QVector3D p1 = _vertices[face.getPoints ()[1].getVertexIndex ()];
                  QVector3D p2 = _vertices[face.getPoints ()[2].getVertexIndex ()];
                  QVector3D n = QVector3D::crossProduct (p1 - p0, p2 - p0);

                  _normals.push_back (n);
                  group->setNormalIndex (j, _normals.size () - 1);
                }
            }
        }

      //
      // Sanity check
      //
      Q_ASSERT (!_vertices.isEmpty ());
      Q_ASSERT (!_normals.isEmpty ());

      foreach (const GroupPtr& group, _groups)
        {
          foreach (const Face& face, group->getFaces ())
            {
              Q_ASSERT (!face.getPoints ().isEmpty ());

              foreach (const Point& point, face.getPoints ())
                {
                  Q_UNUSED (point);

                  Q_ASSERT (point.getVertexIndex () >= -1 &&
                            point.getVertexIndex () < _vertices.size ());
                  Q_ASSERT (point.getNormalIndex () >= -1 &&
                            point.getNormalIndex () < _normals.size ());
                  Q_ASSERT (point.getTextureIndex () >= -1 &&
                            point.getTextureIndex () < _textures.size ());
                }
            }
        }

      //
      // Load material
      //
      if (!material_library.isEmpty ())
        {
          QStringList p = path.split ('/');
          Q_ASSERT (!p.isEmpty ());
          p[p.size () - 1] = material_library;

          loadMaterial (p.join ('/'));
        }

      //
      // Compute bounding box
      //
      _bounding_box.first = QVector3D (std::numeric_limits<float>::max (),
                                       std::numeric_limits<float>::max (),
                                       std::numeric_limits<float>::max ());
      _bounding_box.second = QVector3D (-std::numeric_limits<float>::max (),
                                        -std::numeric_limits<float>::max (),
                                        -std::numeric_limits<float>::max ());

      foreach (const QVector3D& v, _vertices)
        {
          _bounding_box.first.setX (qMin (_bounding_box.first.x (), v.x ()));
          _bounding_box.first.setY (qMin (_bounding_box.first.y (), v.y ()));
          _bounding_box.first.setZ (qMin (_bounding_box.first.z (), v.z ()));

          _bounding_box.second.setX (qMax (_bounding_box.second.x (), v.x ()));
          _bounding_box.second.setY (qMax (_bounding_box.second.y (), v.y ()));
          _bounding_box.second.setZ (qMax (_bounding_box.second.z (), v.z ()));
        }
    }

    /*! Get material by name */
    const Material& Model::getMaterial (const QString& name) const
    {
      MaterialMap::const_iterator pos = _materials.find (name);
      Q_ASSERT (pos != _materials.end () && "Material map corrupted.");

      return pos.value ();
    }

    /*! Load material library */
    void Model::loadMaterial (const QString& path)
    {
      QString content = Tools::loadResource<QString> (path);
      QTextStream file (&content, QIODevice::ReadOnly);

      Material material;

      for (QString line=file.readLine ().trimmed (); !line.isNull (); line=file.readLine ().trimmed ())
        {
          QTextStream in (&line, QIODevice::ReadOnly);

          QString tag;
          in >> tag;
          tag = tag.toLower ();

          //
          // Related material library
          //
          if (tag == "newmtl")
            {
              if (!material.getName ().isEmpty ())
                _materials.insert (material.getName (), material);
              material = Material ();

              QString name;
              in >> name;
              material.setName (name);
            }

          //
          // Ambient
          //
          else if (tag == "ka")
            {
              QString x, y, z;
              in >> x >> y >> z;
              material.setAmbient (toVector3d (x, y, z));
            }

          //
          // Diffuse
          //
          else if (tag == "kd")
            {
              QString x, y, z;
              in >> x >> y >> z;
              material.setDiffuse (toVector3d (x, y, z));
            }

          //
          // Specular
          //
          else if (tag == "ks")
            {
              QString x, y, z;
              in >> x >> y >> z;
              material.setSpecular (toVector3d (x, y, z));
            }

          //
          // Dissolved
          //
          else if (tag == "d" || tag == "tr")
            {
              QString d;
              in >> d;
              material.setDissolved (toReal (d));
            }

          //
          // Specular exponent
          //
          else if (tag == "ns")
            {
              QString ns;
              in >> ns;
              material.setSpecularExponent (toReal (ns));
            }

          //
          // Optical density
          //
          else if (tag == "ni")
            {
              QString ni;
              in >> ni;
              material.setOpticalDensity (toReal (ni));
            }

          //
          // Texture
          //
          else if ( tag == "map_kd" ||
                    tag == "map_ka" ||
                    tag == "map_bump" )
            {
              QString texture;
              in >> texture;
              material.setTexture (texture);
            }
        }

      if (!material.getName ().isEmpty ())
        _materials.insert (material.getName (), material);
    }


    /*! Destructor */
    Model::~Model ()
    {
    }

    /* Convert string into a face point */
    Point Model::toPoint (const QString& t) const // throws Exception
    {
      QStringList parts = t.split ('/', QString::KeepEmptyParts);
      if (parts.isEmpty ())
        throw Exception (QObject::tr ("Invalid face definition"));

      int vertex_index = -1;
      if (parts.size () > 0 && !parts[0].isEmpty ())
        {
          vertex_index = toInt (parts[0]);
          if (vertex_index >= 0)
            vertex_index -= 1;
          else
            vertex_index = _vertices.size () + vertex_index;
        }

      int texture_index = -1;
      if (parts.size () > 1 && !parts[1].isEmpty ())
        {
          texture_index = toInt (parts[1]);
          if (texture_index >= 0)
            texture_index -= 1;
          else
            texture_index = _textures.size () + texture_index;
        }

      int normal_index = -1;
      if (parts.size () > 2 && !parts[2].isEmpty ())
        {
          normal_index = toInt (parts[2]);
          if (normal_index >= 0)
            normal_index -= 1;
          else
            normal_index = _normals.size () + normal_index;
        }

      return Point (vertex_index, normal_index, texture_index);
    }

  }
}
