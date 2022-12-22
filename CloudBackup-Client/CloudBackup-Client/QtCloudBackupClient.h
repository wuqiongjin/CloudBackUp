#pragma once

#include <QtWidgets/QMainWindow>
#include <qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include "ui_QtCloudBackupClient.h"
#include "Monitor.hpp"

class QtCloudBackupClient : public QMainWindow
{
    Q_OBJECT

public:
    QtCloudBackupClient(QWidget *parent = nullptr);
    ~QtCloudBackupClient();
	void browse_on_clicked();
	void apply_on_clicked();
	void delete_on_clicked();
	//void exit_on_clicked();
	bool CheckPathValid(const std::string& path);
	bool Backup_Action(bool needCheck);

private:
    Ui::QtCloudBackupClientClass ui;
	CloudBackup::Monitor mon;
};
