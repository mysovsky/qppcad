#ifndef QPPCAD_PLUGIN_PARAM_EDITOR
#define QPPCAD_PLUGIN_PARAM_EDITOR

#include <qppcad/python/python_plugin.hpp>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAbstractTableModel>
#include <QTableView>

namespace qpp {

  namespace cad {
    
    class plugin_param_model_t : public QAbstractTableModel {

      Q_OBJECT

    private:

      std::shared_ptr<python_plugin_t> plugin;

      int rowCount(const QModelIndex &parent) const;
      int columnCount(const QModelIndex &parent) const;
      QVariant data(const QModelIndex &index, int role) const;
      QVariant headerData(int section, Qt::Orientation orientation, int role) const;
      Qt::ItemFlags flags(const QModelIndex &index) const;
      bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
      
    public:

      void bind(std::shared_ptr<python_plugin_t> p);
      void unbind();

    };

    // -----------------------------------------------------------
    
    class plugin_param_editor_t : public QDialog {
      
      Q_OBJECT
      
    public:      
      
      QVBoxLayout *main_lt;
      QHBoxLayout *button_lt;
      QPushButton *ok_button;
      QPushButton *cancel_button;
      QPushButton *browse_btn;

      QTableView *param_tbl;
      plugin_param_model_t *param_mdl;
      
      std::shared_ptr<python_plugin_t> plugin;
      int ifile;

      plugin_param_editor_t(std::shared_ptr<python_plugin_t> p);
      std::vector<std::shared_ptr<geom_view_t> > list_ws_items();
											     
    public slots:

      void ok_button_clicked();
      void cancel_button_clicked();
      void browse_button_clicked();
      
    };
    
  } // namespace qpp::cad

} // namespace qpp

#endif
