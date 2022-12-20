#include "QtCloudBackupClient.h"

QtCloudBackupClient::QtCloudBackupClient(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	//this->setWindowFlags(Qt::FramelessWindowHint);//隐藏窗口栏

	connect(ui.bt_apply, &QPushButton::clicked, this, &QtCloudBackupClient::apply_on_clicked);
	connect(ui.bt_exit, &QPushButton::clicked, this, &QtCloudBackupClient::exit_on_clicked);
}

QtCloudBackupClient::~QtCloudBackupClient()
{}

bool QtCloudBackupClient::CheckPathValid(const std::string& path)
{
	CloudBackup::FileUtil fu(path);
	return fu.Exists() && fs::is_directory(path);
}

bool QtCloudBackupClient::Backup_Action(bool needCheck = true)
{
	QString q_path = ui.le_path->text();
	std::string path = q_path.toLocal8Bit();

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

	//调用云备份功能前, 必须保证路径的有效
	//(虽然非法路径进去也不会被备份, 非法路径其目录下的文件时就会出问题, 然后返回)
	CloudBackup::Backup bp(path);
	bp.RunModule();
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
	}
}

void QtCloudBackupClient::exit_on_clicked()
{
	Backup_Action(false);	//关闭的时候不需要检查路径(况且即便是非法路径, 我们的程序也不会出问题, apply里面要检查的原因是给用户报出警告)
	QApplication* app;
	app->exit();
}