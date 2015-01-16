/*
 * hip_explorer_tag_selector.cpp - Widget for selecting filter tags
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPExplorerTagSelector.h"
#include "ui_hip_explorer_tagselector.h"

#include "database/HIPDatabase.h"

#include <QAbstractItemModel>

namespace HIP {
  namespace Explorer {

    //#**********************************************************************
    // CLASS HIP::Explorer::TagSelectorModel
    //#**********************************************************************

    /*
     * Model for the tag selector
     */
    class TagSelectorModel : public QAbstractItemModel
    {
    public:
      TagSelectorModel (Database::Database* database, QObject* parent);
      virtual ~TagSelectorModel ();

      virtual int columnCount (const QModelIndex& parent) const;
      virtual int rowCount (const QModelIndex& parent) const;

      virtual QModelIndex index (int row, int column, const QModelIndex& parent) const;
      virtual QModelIndex parent (const QModelIndex& index) const;
      virtual Qt::ItemFlags flags (const QModelIndex& index) const;

      virtual QVariant data (const QModelIndex& index, int role) const;

    private:
      Database::Database* _database;
    };

    /* Constructor */
    TagSelectorModel::TagSelectorModel (Database::Database* database, QObject* parent)
      : QAbstractItemModel (parent),
        _database (database)
    {
    }

    /*! Destructor */
    TagSelectorModel::~TagSelectorModel ()
    {
    }

    int TagSelectorModel::columnCount (const QModelIndex& parent) const
    {
      Q_UNUSED (parent);
      return 1;
    }

    int TagSelectorModel::rowCount (const QModelIndex& parent) const
    {
      return !parent.isValid () ? _database->getTags ().size () : 0;
    }

    QModelIndex TagSelectorModel::index (int row, int column, const QModelIndex& parent) const
    {
      Q_UNUSED (parent);
      return createIndex (row, column);
    }

    QModelIndex TagSelectorModel::parent (const QModelIndex& index) const
    {
      Q_UNUSED (index);
      return QModelIndex ();
    }

    Qt::ItemFlags TagSelectorModel::flags (const QModelIndex& index) const
    {
      Q_UNUSED (index);
      return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    QVariant TagSelectorModel::data (const QModelIndex& index, int role) const
    {
      QVariant result;

      const QList<QString>& tags = _database->getTags ();

      if (index.isValid () && index.row () < tags.size ())
        {
          const QString& tag = tags[index.row ()];

          switch (role)
            {
            case Qt::DisplayRole:
              result = qVariantFromValue (tag);
              break;
            }
        }

      return result;
    }



    //#**********************************************************************
    // CLASS HIP::Explorer::TagSelector
    //#**********************************************************************

    /* Constructor */
    TagSelector::TagSelector (Database::Database* database, QWidget* parent)
      : QWidget(parent),
      _ui (new Ui::HIP_Explorer_TagSelector)
    {
      Q_UNUSED (database);
      _ui->setupUi (this);

      _ui->_input->setModel (new TagSelectorModel (database, this));
    }

    /*! Destructor */
    TagSelector::~TagSelector ()
    {
      delete _ui;
    }

  }
}
