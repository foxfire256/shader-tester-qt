#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>

class QTimer;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;

class gl_widget;

class main_window : public QMainWindow
{
	Q_OBJECT

public:
	main_window(QWidget *parent = nullptr);
	virtual ~main_window();

public slots:
	void update_fps(int fps);

private:
	gl_widget *glw;
	QTimer *gfx_timer;
	QHBoxLayout *main_layout;
	QVBoxLayout *right_layout;
	QLabel *fps_lbl;
	QWidget *main_widget;
};

#endif
