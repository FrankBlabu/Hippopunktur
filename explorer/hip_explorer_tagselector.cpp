/*
 * hip_explorer_tag_selector.cpp - Widget for selecting filter tags
 *
 * Frank Blankenburg, Jan. 2015
 */

#include "HIPExplorerTagSelector.h"
#include "ui_hip_explorer_tagselector.h"

#include "database/HIPDatabase.h"

#include <QAbstractItemDelegate>
#include <QAbstractItemModel>
#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QPalette>
#include <QRectF>
#include <QStyleOptionViewItem>

namespace HIP {
  namespace Explorer {

    //#**********************************************************************
    // CLASS HIP::Explorer::TagSelectorModel
    //#**********************************************************************

    /* Constructor */
    TagSelectorModel::TagSelectorModel (Database::Database* database, QObject* parent)
      : QAbstractItemModel (parent),
        _database (database)
    {
      connect (database, &Database::Database::databaseChanged, this, &TagSelectorModel::onDatabaseChanged);
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

    void TagSelectorModel::onDatabaseChanged (Database::Database::Reason_t reason, const QString& id)
    {
      Q_UNUSED (id);

      switch (reason)
        {
        case Database::Database::Reason::DATA:
        case Database::Database::Reason::POINT:
          beginResetModel ();
          endResetModel ();
          break;

        case Database::Database::Reason::SELECTION:
        case Database::Database::Reason::FILTER:
        case Database::Database::Reason::VISIBLE_IMAGE:
          break;
        }
    }



    //#**********************************************************************
    // CLASS HIP::Explorer::TagSelectorDelegate
    //#**********************************************************************

    /*!
     * Delegate for the tag selector
     */
    class TagSelectorDelegate : public QAbstractItemDelegate
    {
    public:
      TagSelectorDelegate (QObject* parent);
      virtual ~TagSelectorDelegate ();

      virtual void paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
      virtual QSize	sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const;

    private:
      static const int MARGIN = 2;
      static const int SPACING = 2;
    };

    TagSelectorDelegate::TagSelectorDelegate (QObject* parent)
      : QAbstractItemDelegate (parent)
    {
    }

    TagSelectorDelegate::~TagSelectorDelegate ()
    {
    }

    void TagSelectorDelegate::paint (QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
      QString tag = index.model ()->data (index, Qt::DisplayRole).toString ();

      if (option.state.testFlag (QStyle::State_Selected))
        {
          painter->setPen (QPen ());
          painter->setBrush (option.palette.highlight ());
          painter->drawRect (option.rect);
        }

      QFontMetrics metrics (QApplication::font ());
      QRect metrics_rect = metrics.boundingRect (tag);

      QRect rect = option.rect;
      rect.setWidth (metrics_rect.width () + 4 * MARGIN);

      QRect background_rect (rect.left (), rect.top () + SPACING / 2,
                             rect.width (), rect.height () - SPACING);

      painter->setPen (QColor (0x70, 0xc0, 0xd0));
      painter->setBrush (QColor (0x70, 0xc0, 0xd0));
      painter->drawRoundedRect (background_rect, MARGIN, MARGIN);

      if (option.state.testFlag (QStyle::State_Selected))
        painter->setPen (option.palette.highlightedText ().color ());
      else
        painter->setPen (option.palette.text ().color ());

      painter->drawText (rect, tag, Qt::AlignHCenter | Qt::AlignVCenter);
    }

    QSize	TagSelectorDelegate::sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
      Q_UNUSED (option);

      QFontMetrics metrics (QApplication::font ());
      QRect rect = metrics.boundingRect (index.model ()->data (index, Qt::DisplayRole).toString ());

      rect.setWidth (rect.width () + 2 * MARGIN);
      rect.setHeight (rect.height () + 2 * MARGIN + SPACING);

      return rect.size ();
    }


    //#**********************************************************************
    // CLASS HIP::Explorer::TagSelector
    //#**********************************************************************

    /* Constructor */
    TagSelector::TagSelector (Database::Database* database, QWidget* parent)
      : QWidget(parent),
      _ui       (new Ui::HIP_Explorer_TagSelector),
      _database (database)
    {
      Q_UNUSED (database);
      _ui->setupUi (this);

      _ui->_input_w->setModel (new TagSelectorModel (database, this));
      //_ui->_input_w->setItemDelegate (new TagSelectorDelegate (this));

      connect (_ui->_clear_w, SIGNAL (clicked ()), SLOT (onClear ()));
      connect (_ui->_input_w, SIGNAL (currentTextChanged (const QString&)), SLOT (onTextChanged (const QString&)));
      connect (_ui->_input_w, SIGNAL (activated (int)), SLOT (onActivated (int)));
    }

    /*! Destructor */
    TagSelector::~TagSelector ()
    {
      delete _ui;
    }

    /*! Filter text changed */
    void TagSelector::onTextChanged (const QString& text)
    {
      foreach (const Database::Point& point, _database->getPoints ())
        if (point.getSelected () && !point.matches (text))
          _database->deselect (point.getId ());

      if (_database->getFilter () != text)
        _database->setFilter (text);
    }

    /* Item has been selected */
    void TagSelector::onActivated (int index)
    {
      Q_ASSERT (index >= 0 && index < _database->getTags ().size ());
      QString tag = _database->getTags ()[index];

      {
        QSignalBlocker blocker (_ui->_input_w);
        _ui->_input_w->setCurrentText (tag);
      }

      onTextChanged (tag);
    }

    /* Clear tag selector */
    void TagSelector::onClear ()
    {
      {
        QSignalBlocker blocker (_ui->_input_w);
        _ui->_input_w->setCurrentText (QString ());
      }

      onTextChanged (QString ());
    }

  }
}
