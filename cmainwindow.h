#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class cMainWindow; }
QT_END_NAMESPACE

class cMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	cMainWindow(QWidget *parent = nullptr);
	~cMainWindow();

private:
	Ui::cMainWindow *ui;
};
#endif // CMAINWINDOW_H
