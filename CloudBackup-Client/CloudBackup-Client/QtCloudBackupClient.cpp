#include "QtCloudBackupClient.h"

QtCloudBackupClient::QtCloudBackupClient(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	//this->setWindowFlags(Qt::FramelessWindowHint);//隐藏窗口栏

	connect(ui.bt_browse, &QPushButton::clicked, this, &QtCloudBackupClient::browse_on_clicked);
	connect(ui.bt_apply, &QPushButton::clicked, this, &QtCloudBackupClient::apply_on_clicked);
	connect(ui.bt_delete, &QPushButton::clicked, this, &QtCloudBackupClient::delete_on_clicked);
	auto monitored_list = mon.ShowMonitorList();
	for (auto& str : monitored_list)
	{
		QListWidgetItem* item = new QListWidgetItem();
		item->setText(QString().fromLocal8Bit(str.c_str()));
		ui.ltw_showlist->addItem(item);
	}
	//connect(ui.bt_exit, &QPushButton::clicked, this, &QtCloudBackupClient::exit_on_clicked);
}

QtCloudBackupClient::~QtCloudBackupClient()
{}

void QtCloudBackupClient::browse_on_clicked()
{
	QString q_path = QFileDialog::getExistingDirectory(this, tr("Open Directory"), \
		"/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	ui.le_path->setText(q_path);
}

bool QtCloudBackupClient::CheckPathValid(const std::string& path)
{
	CloudBackup::FileUtil fu(path);
	return fu.Exists() && fs::is_directory(path);
}

bool QtCloudBackupClient::Backup_Action(bool needCheck = true)
{
	//编码格式转换
	QString q_path = ui.le_path->text();
	std::string path = q_path.toLocal8Bit();

	//这里的路径我们统一为绝对路径!以防止用户添加相同目录的相对路径和绝对路径
	CloudBackup::FileUtil fu(path);
	path = fu.GetAbsolutePath();
	q_path = q_path.fromLocal8Bit(path.c_str());	//确保用户输入的路径转化为了绝对路径

	if (path.empty()) { return false; }	//不允许输入空目录
	if (needCheck) {
		bool ret = CheckPathValid(path);
		if (!ret) {
			QMessageBox::warning(this,
				tr("Warning!"),
				tr("Invalid Backup Path! Please check if your path is correct!")
			);
			return false;
		}
	}

	//找得着, 说明重复添加了备份目录了, 直接return
	if (mon.Search(path)) {
		QMessageBox::warning(this,
			tr("Warning!"),
			tr("The Backup Directory Already Exists!")
		);
		return false;
	}

	//调用云备份功能前, 必须保证路径的有效
	//(虽然非法路径进去也不会被备份, 非法路径其目录下的文件时就会出问题, 然后返回)

	//CloudBackup::Backup bp(path);
	//bp.RunModule();

	//向ListWidget区域添加一条path路径
	QListWidgetItem* item = new QListWidgetItem();
	item->setText(q_path);
	ui.ltw_showlist->addItem(item);

	mon.Insert(path);
	return true;
}

void QtCloudBackupClient::apply_on_clicked()
{
	bool ret = Backup_Action();
	//只有上面的操作执行了, 才报出下面的Information
	if (ret) {
		QMessageBox::information(this,
			tr("Message:"),
			tr("Complete the backup of the target directory!")
		);
		ui.le_path->clear();	//清空LineEdit内容
	}
}

void QtCloudBackupClient::delete_on_clicked()
{
	//先判断是否选中某一条数据, 没选中禁止删除
	if (!ui.ltw_showlist->currentItem()) {
		QMessageBox::warning(this,
			tr("Warning!"),
			tr("You MUST Select a piece of Data First!")
		);
		return;
	}

	//删除选中行
	int row = ui.ltw_showlist->currentRow();
	QListWidgetItem* item = ui.ltw_showlist->takeItem(row);
	QString q_path = item->text();
	delete item;

	std::string path = q_path.toLocal8Bit();
	CloudBackup::FileUtil fu(path);
	mon.Delete(fu.GetAbsolutePath());	//真正意义上的删除备份目录(解除监控状态), 这里要确保路径是绝对路径!
	QMessageBox::information(this,
		tr("Message:"),
		tr("Successfully Delete the Selected Directory!")
	);
}