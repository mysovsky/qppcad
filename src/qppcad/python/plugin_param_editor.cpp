#include <qppcad/python/plugin_param_editor.hpp>
#include <qppcad/core/app_state.hpp>
#include <QString>
#include <QHeaderView>
#include <QCoreApplication>
#include <QTableWidgetItem>
#include <QFileDialog>

using namespace qpp;
using namespace qpp::cad;

int plugin_param_model_t::rowCount(const QModelIndex &parent) const {
  if (plugin) return plugin -> params.size();
  else return 0;
}

int plugin_param_model_t::columnCount(const QModelIndex &parent) const {
  return 3;
}

QVariant plugin_param_model_t::data(const QModelIndex &index, int role) const {

  if (!plugin) return QVariant();

  if (role == Qt::TextAlignmentRole) {
      return Qt::AlignCenter;
    }

  int i = index.row();
  int j = index.column();
  
  if (role == Qt::DisplayRole || role == Qt::EditRole) {

    switch (j) {

    case 0 :
      return QString(plugin->params[i]->name.c_str());
      break;
      
    case 1 :
      return QString(type_data::type_name[plugin->params[i]->type].c_str());
      break;
      
    case 2 :
      if (plugin -> params[i] -> type == type_bool)
	return QVariant();
      else
	return QString(plugin->params[i]->sval.c_str());
      break;
      
    default:
      return QVariant();
          break;
    }
  }
  else if (j == 2 && plugin -> params[i] -> type == type_bool
	   && role == Qt::CheckStateRole) {
    //return Qt::Checked or Qt::Unchecked here
    if (std::get<bool>(plugin -> params[i] -> value))
      return Qt::Checked;
    else
      return Qt::Unchecked;
  }
  return QVariant();  
}

QVariant plugin_param_model_t::headerData(int section,
                                           Qt::Orientation orientation,
                                           int role) const {

  if (role == Qt::DisplayRole) {

    if (orientation == Qt::Horizontal) {
      switch (section) {
	
      case 0:
	return tr("Parameter");
	break;
	
      case 1:
	return tr("Type");
	break;
	
      case 2:
	return tr("Value");
	break;
	
      default:
	return QString("");
	
      }
    }
  }
  return QVariant();
}

Qt::ItemFlags plugin_param_model_t::flags(const QModelIndex &index) const {

  Qt::ItemFlags flags = Qt::ItemFlag::ItemIsEnabled;
  if (index.column() == 2) {
    if ( plugin -> params[index.row()] -> type == type_bool )
      flags = flags | Qt::ItemFlag::ItemIsUserCheckable;
    else
      flags = flags | Qt::ItemFlag::ItemIsEditable;
  }
  return flags;

}

bool plugin_param_model_t::setData(const QModelIndex &index, const QVariant &value, int role) {
  int i = index.row();
  int j = index.column();

  if (j == 2 && role == Qt::EditRole) {

    if (value.type() == QVariant::String) {
      
      plugin -> params[i] -> sval = value.toString().toStdString();
      return true;
    }

  }
  
  if (role == Qt::CheckStateRole && j ==2 &&
      plugin -> params[i] -> type == type_bool ){    
    if ((Qt::CheckState)value.toInt() == Qt::Checked) {    
      //user has checked item
      plugin -> params[i] -> value = true;
      plugin -> params[i] -> sval = "true";
      return true;
    }
    else {
      //user has unchecked item
      plugin -> params[i] -> value = false;
      plugin -> params[i] -> sval = "false";
      return true;
    }
  }
  return QAbstractTableModel::setData(index, value, role);
}



void plugin_param_model_t::bind(std::shared_ptr<python_plugin_t> p){
  plugin = p;
  for (auto prm : plugin -> params)
    prm -> sval = prm -> default_sval;
}

void plugin_param_model_t::unbind(){
  plugin = nullptr;
}

std::vector<std::shared_ptr<geom_view_t> > plugin_param_editor_t::list_ws_items(){
  app_state_t *astate = app_state_t::get_inst();
  auto cur_ws = astate -> ws_mgr ->  get_cur_ws();
  std::vector<std::shared_ptr<geom_view_t> > list;
  for (auto it : cur_ws -> m_ws_items){
    auto gv = std::static_pointer_cast<geom_view_t>(it);
    if (!gv || !gv->m_geom) continue;
    list.push_back(gv);
  }
  return list;
}

plugin_param_editor_t::plugin_param_editor_t(std::shared_ptr<python_plugin_t> p){

  plugin = p;
  app_state_t *astate = app_state_t::get_inst();
  
  setWindowTitle(tr(plugin -> plug_name.c_str()));
    
  main_lt = new QVBoxLayout;
  button_lt = new QHBoxLayout;

  ok_button = new QPushButton(tr("Ok"));
  ok_button->setFixedWidth(astate->size_guide.common_button_fixed_w());
  cancel_button = new QPushButton(tr("Cancel"));
  cancel_button->setFixedWidth(astate->size_guide.common_button_fixed_w());

  button_lt->addStretch();
  button_lt->addWidget(ok_button, 0, Qt::AlignCenter);
  button_lt->addWidget(cancel_button, 0, Qt::AlignCenter);
  button_lt->addStretch();

  //auto spoil = new qspoiler_widget_t(tr("Edit plugin parameters:"), nullptr, false);
  //auto spoil_lt = new QHBoxLayout;
  //spoil -> add_content_layout(spoil_lt);
  param_tbl = new QTableView;
  //spoil_lt -> addWidget(param_tbl);
  
  param_mdl = new plugin_param_model_t;
  param_mdl -> bind(plugin);
  param_tbl -> setModel(param_mdl);

  param_tbl -> setFixedWidth(650);
  param_tbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  param_tbl->verticalHeader()->hide();
  param_tbl->horizontalHeader()->setStretchLastSection(true);
  //param_tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  for (int i=0; i < plugin -> params.size(); i++)
    if (plugin -> params[i] -> type == type_qpp_geometry){

      auto cmb_itm = new QComboBox;
      auto geoms = list_ws_items();
      for (auto g:geoms)
	cmb_itm -> addItem(tr(g->m_name.c_str()));
      QObject::connect( cmb_itm,
			static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			[this, i, geoms](int idx){
			  this -> plugin -> params[i] -> value = geoms[idx] -> m_geom;
			  //astate -> tlog("plug param edit combo : {}",
			  //		 std::get<std::shared_ptr<xgeometry<float, periodic_cell<float> > > >(this -> plugin -> params[i] -> value) -> nat());
			});
      plugin -> params[i] -> value = geoms[0] -> m_geom;
      
      auto I = param_mdl -> index(i,2);
      param_tbl -> setIndexWidget(I, cmb_itm);
    }
  
  ifile = -1;
  for (int ii = 0; ii < plugin -> params.size(); ii++)
    if (plugin -> params[ii] -> browse != ""){
      ifile = ii;
      break;
    }

  auto descr = new qspoiler_widget_t(tr("Description"),nullptr,true);
  auto descr_lt = new QVBoxLayout;
  descr_lt -> addWidget(new QLabel(tr(plugin->description.c_str())));
  descr -> add_content_layout(descr_lt);
  descr -> setFixedWidth(650);
  
  main_lt -> addWidget(descr);
  main_lt -> addWidget(param_tbl);

  if (ifile >=0){
    browse_btn = new QPushButton(tr("Browse"));
    browse_btn -> setFixedWidth(astate->size_guide.common_button_fixed_w());
    auto browse_lt = new QHBoxLayout;
    std::string br_label = "Use this button to browse the file \"";
    br_label = br_label + plugin -> params[ifile] -> name + "\"";
    browse_lt -> addWidget(browse_btn);
    browse_lt -> addWidget(new QLabel(tr(br_label.c_str())));
    main_lt -> addLayout(browse_lt);
  }
  
  main_lt -> addLayout(button_lt);
  setLayout(main_lt);

  // param_tbl->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
  // param_tbl->setFocusPolicy(Qt::NoFocus);
  //->setFixedWidth(astate->size_guide.obj_insp_table_w());
  //->setShowGrid(false);
  /*
  app_state_t::get_inst() -> tlog("columns: {} {} {} {}:", param_tbl->columnWidth(0),
				  param_tbl->columnWidth(1),param_tbl->columnWidth(2),
				  param_tbl->columnWidth(3));
  int tbl_width = 0;
  for (int i=0; i<4; i++)
    tbl_width +=  param_tbl->columnWidth(i);
  
  
  
  if (ifile > -1){
    auto I = param_mdl -> index(ifile,3);
    param_tbl -> setIndexWidget(I, bc_btn);
  }
  
  app_state_t::get_inst() -> tlog("desription: {}",plugin->description);

  // Fuck fuck fuck QT !!!!!
  // ---------------------------------------------
  param_tbl -> horizontalHeader() -> setSectionResizeMode(2, QHeaderView::Stretch);
  param_tbl -> horizontalHeader() -> setSectionResizeMode(0, QHeaderView::ResizeToContents);
  param_tbl -> horizontalHeader() -> setSectionResizeMode(1, QHeaderView::ResizeToContents);
  param_tbl -> horizontalHeader() -> setSectionResizeMode(3, QHeaderView::ResizeToContents);
  param_tbl -> resizeColumnToContents(3);
  //param_tbl -> setColumnWidth(2,   tbl_width/2);
  //param_tbl -> setColumnWidth(1,   tbl_width/5);
  //param_tbl -> setColumnWidth(2, 2*tbl_width/5);
  */

  connect(ok_button, &QPushButton::clicked,
          this, &plugin_param_editor_t::ok_button_clicked);

  connect(cancel_button, &QPushButton::clicked,
          this, &plugin_param_editor_t::cancel_button_clicked);

  if (ifile >=0 )
    connect(browse_btn, &QPushButton::clicked,
	    this, &plugin_param_editor_t::browse_button_clicked);
}

void plugin_param_editor_t::ok_button_clicked(){
  param_mdl -> unbind();
  accept();
}

void plugin_param_editor_t::cancel_button_clicked(){
  param_mdl -> unbind();
  reject();
}

void plugin_param_editor_t::browse_button_clicked(){
  //app_state_t::get_inst() -> tlog("Browse btn clicked");
  std::string s;
  if ( plugin -> params[ifile] -> browse == "save" )
    s = QFileDialog::getSaveFileName(this,
				     "Select file",
				     plugin -> params[ifile] -> sval.c_str(),
				     "*").toStdString();
  else if ( plugin -> params[ifile] -> browse == "open" )
    s = QFileDialog::getOpenFileName(this,
				     "Select file",
				     plugin -> params[ifile] -> sval.c_str(),
				     "*").toStdString();
  plugin -> params[ifile] -> sval = s;
}

